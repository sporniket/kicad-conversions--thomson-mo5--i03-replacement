# SPDX-License-Identifier: CC0-1.0
# (c) 2024 David SPORN
# experimental -- 2024-11-04

from amaranth import * #replace with no star import/alias : `import amaranth as a` or something like that.

class PromColorSimulator(Elaboratable): # old style now, migrate to Component

    def elaborate(self,platform):
        m = Module()

        # input ports
        # -- check actual queries
        p3, p4, p5, p6, p7 = platform.getRessource("p3", inverted),
        platform.getRessource("p4"),
        platform.getRessource("p5"),
        platform.getRessource("p6"),
        platform.getRessource("p7"),

        # i/o/q ports
        # -- check actual queries
        p12, p14, p15, p16, p17, p18, p19 = platform.getRessource("p12", output, inverted),
        platform.getRessource("p14", output),
        platform.getRessource("p15", output),
        platform.getRessource("p16", output),
        platform.getRessource("p17", output),
        platform.getRessource("p18", output),
        platform.getRessource("p19", output),



        # inputs
        supLT = Signal() ; #<-- active low
        pastel, bleu, vert, rouge = Signal(), Signal(), Signal(), Signal() ;

        # outputs
        R=Signal(2)
        V=Signal(2)
        B=Signal(2)
        noirIncr = Signal() # --> active low

        # input port --> inputs
        m.d.comb += [
            supLT.eq(p3),
            pastel.eq(p4), 
            bleu.eq(p5), 
            vert.eq(p6), 
            rouge.eq(p7), 
        ]


        # outputs --> i/o/q ports
        m.d.comb += [
            p12.eq(noirIncr),
            p14.eq(B[1]),
            p15.eq(B[0]),
            p16.eq(V[1]),
            p17.eq(V[0]),
            p18.eq(B[1]),
            p19.eq(B[0]),
        ]

        # Formulas

        # -- Noir incrustation
        m.d.comb += noirIncr.eq(supLT | (~Cat(rouge, vert, bleu, pastel).all()))

        # -- RED
        # 0b11 when rouge
        # else 0b10 when pastel
        # else 0b00
        with m.If(rouge):
            m.d.comb += R.eq(Cat(1,1))
        with m.Elif(pastel):
            m.d.comb += R.eq(Cat(0,1))
        with m.Else():
            m.d.comb += R.eq(Cat(0,0))

        # -- GREEN
        # 0b10 when (pastel, bleu, vert, rouge) a.k.a orange
        # else 0b11 when vert
        # else 0b10 when pastel
        # else 0b00
        with m.If((Cat(rouge, vert, bleu, pastel).all()):
            m.d.comb += V.eq(Cat(0,1))
        with m.Elif(vert):
            m.d.comb += V.eq(Cat(1,1))
        with m.Elif(pastel):
            m.d.comb += V.eq(Cat(0,1))
        with m.Else():
            m.d.comb += V.eq(Cat(0,0))

#include <gtest/gtest.h>
#include "mathml2omml.h"

TEST(Mathml2OmmlTest, SimpleSuperscript)
{
    std::string mml =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">"
        "<semantics>"
        "<mrow><msup><mi>x</mi><mn>2</mn></msup></mrow>"
        "<annotation encoding=\"application/x-tex\">x^2</annotation>"
        "</semantics>"
        "</math>";

    std::string omml = MathmlToOmml::convert(mml);

    EXPECT_TRUE(omml.find("<m:oMath") != std::string::npos) << "Must produce m:oMath element";
    EXPECT_TRUE(omml.find("<m:sSup>") != std::string::npos) << "Superscript must produce m:sSup";
    EXPECT_TRUE(omml.find("<m:e>") != std::string::npos) << "Must have m:e for base";
    EXPECT_TRUE(omml.find("<m:sup>") != std::string::npos) << "Must have m:sup for exponent";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "Base text 'x' must be present";
    EXPECT_TRUE(omml.find(">2<") != std::string::npos) << "Exponent text '2' must be present";
    EXPECT_TRUE(omml.find("x^2") == std::string::npos) << "Raw TeX must not appear in OMML output";
}

TEST(Mathml2OmmlTest, Fraction)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><mfrac><mi>a</mi><mi>b</mi></mfrac></mrow>"
        "<annotation encoding=\"application/x-tex\">\\frac{a}{b}</annotation>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml);

    EXPECT_TRUE(omml.find("<m:f>") != std::string::npos) << "Fraction must produce m:f";
    EXPECT_TRUE(omml.find("<m:num>") != std::string::npos) << "Must have m:num for numerator";
    EXPECT_TRUE(omml.find("<m:den>") != std::string::npos) << "Must have m:den for denominator";
    EXPECT_TRUE(omml.find(">a<") != std::string::npos) << "Numerator text 'a' must be present";
    EXPECT_TRUE(omml.find(">b<") != std::string::npos) << "Denominator text 'b' must be present";
}

TEST(Mathml2OmmlTest, SquareRoot)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><msqrt><mi>x</mi></msqrt></mrow>"
        "<annotation encoding=\"application/x-tex\">\\sqrt{x}</annotation>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml);

    EXPECT_TRUE(omml.find("<m:rad>") != std::string::npos) << "Square root must produce m:rad";
    EXPECT_TRUE(omml.find("<m:deg/>") != std::string::npos) << "m:deg must be empty for sqrt";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "Radicand text 'x' must be present";
}

TEST(Mathml2OmmlTest, NthRootReordersChildren)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><mroot><mi>x</mi><mn>3</mn></mroot></mrow>"
        "<annotation encoding=\"application/x-tex\">\\sqrt[3]{x}</annotation>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml);

    size_t degPos = omml.find("<m:deg>");
    size_t ePos = omml.find("<m:e>");
    EXPECT_NE(degPos, std::string::npos) << "m:deg must be present";
    EXPECT_NE(ePos, std::string::npos) << "m:e must be present";
    EXPECT_LT(degPos, ePos) << "m:deg must precede m:e in OMML output";
    EXPECT_TRUE(omml.find(">3<") != std::string::npos) << "Degree value '3' must be present";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "Radicand 'x' must be present";
}

TEST(Mathml2OmmlTest, Subscript)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><msub><mi>a</mi><mn>1</mn></msub></mrow>"
        "<annotation encoding=\"application/x-tex\">a_1</annotation>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml);

    EXPECT_TRUE(omml.find("<m:sSub>") != std::string::npos) << "Subscript must produce m:sSub";
    EXPECT_TRUE(omml.find("<m:sub>") != std::string::npos) << "Must have m:sub element";
    EXPECT_TRUE(omml.find(">a<") != std::string::npos) << "Base 'a' must be present";
    EXPECT_TRUE(omml.find(">1<") != std::string::npos) << "Subscript '1' must be present";
}

TEST(Mathml2OmmlTest, SubSup)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><msubsup><mi>A</mi><mn>1</mn><mn>2</mn></msubsup></mrow>"
        "<annotation encoding=\"application/x-tex\">A_1^2</annotation>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml);

    EXPECT_TRUE(omml.find("<m:sSubSup>") != std::string::npos) << "SubSup must produce m:sSubSup";
    EXPECT_TRUE(omml.find("<m:sub>") != std::string::npos) << "Must have m:sub element";
    EXPECT_TRUE(omml.find("<m:sup>") != std::string::npos) << "Must have m:sup element";
    EXPECT_TRUE(omml.find(">A<") != std::string::npos) << "Base 'A' must be present";
    EXPECT_TRUE(omml.find(">1<") != std::string::npos) << "Subscript '1' must be present";
    EXPECT_TRUE(omml.find(">2<") != std::string::npos) << "Superscript '2' must be present";
}

TEST(Mathml2OmmlTest, ItalicStyle)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><mi>x</mi></mrow>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml);

    EXPECT_TRUE(omml.find("<m:sty m:val=\"i\"/>") != std::string::npos) << "mi must use italic style";
}

TEST(Mathml2OmmlTest, OverAndUnderLimits)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><mover><mi>x</mi><mo>&#xAF;</mo></mover></mrow>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml);

    EXPECT_TRUE(omml.find("<m:limUpp>") != std::string::npos) << "mover must produce m:limUpp";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "Base 'x' must be present";
}

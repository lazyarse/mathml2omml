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

    std::string omml = MathmlToOmml::convert(mml).value();

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

    std::string omml = MathmlToOmml::convert(mml).value();

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

    std::string omml = MathmlToOmml::convert(mml).value();

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

    std::string omml = MathmlToOmml::convert(mml).value();

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

    std::string omml = MathmlToOmml::convert(mml).value();

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

    std::string omml = MathmlToOmml::convert(mml).value();

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

    std::string omml = MathmlToOmml::convert(mml).value();

    EXPECT_TRUE(omml.find("<m:sty m:val=\"i\"/>") != std::string::npos) << "mi must use italic style";
}

TEST(Mathml2OmmlTest, OverAndUnderLimits)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><mover><mi>x</mi><mo>&#xAF;</mo></mover></mrow>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml).value();

    EXPECT_TRUE(omml.find("<m:limUpp>") != std::string::npos) << "mover must produce m:limUpp";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "Base 'x' must be present";
}

// ── Reverse conversion tests (OMML → MathML) ─────────────────────────────

TEST(Omml2MathmlTest, SimpleSuperscript)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:sSup><m:e><m:r><m:t>x</m:t></m:r></m:e>"
        "<m:sup><m:r><m:sty m:val=\"p\"/><m:t>2</m:t></m:r></m:sup></m:sSup>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<msup>") != std::string::npos) << "Must produce msup";
    EXPECT_TRUE(mml.find(">x<") != std::string::npos) << "Base 'x' must be present";
    EXPECT_TRUE(mml.find(">2<") != std::string::npos) << "Exponent '2' must be present";
    EXPECT_TRUE(mml.find("<mn>2</mn>") != std::string::npos) << "2 should be mn (number)";
}

TEST(Omml2MathmlTest, Fraction)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:f>"
        "<m:num><m:r><m:t>a</m:t></m:r></m:num>"
        "<m:den><m:r><m:t>b</m:t></m:r></m:den>"
        "</m:f>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<mfrac>") != std::string::npos) << "Must produce mfrac";
    EXPECT_TRUE(mml.find(">a<") != std::string::npos) << "Num 'a' must be present";
    EXPECT_TRUE(mml.find(">b<") != std::string::npos) << "Den 'b' must be present";
}

TEST(Omml2MathmlTest, SquareRoot)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:rad><m:deg/><m:e><m:r><m:t>x</m:t></m:r></m:e></m:rad>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<msqrt>") != std::string::npos) << "Must produce msqrt";
    EXPECT_TRUE(mml.find(">x<") != std::string::npos) << "Radicand 'x' must be present";
}

TEST(Omml2MathmlTest, NthRoot)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:rad><m:deg><m:r><m:t>3</m:t></m:r></m:deg>"
        "<m:e><m:r><m:t>x</m:t></m:r></m:e></m:rad>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<mroot>") != std::string::npos) << "Must produce mroot";
    EXPECT_TRUE(mml.find(">x<") != std::string::npos) << "Radicand 'x' must be present";
    EXPECT_TRUE(mml.find(">3<") != std::string::npos) << "Degree '3' must be present";
}

TEST(Omml2MathmlTest, Subscript)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:sSub><m:e><m:r><m:t>a</m:t></m:r></m:e>"
        "<m:sub><m:r><m:t>1</m:t></m:r></m:sub></m:sSub>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<msub>") != std::string::npos) << "Must produce msub";
    EXPECT_TRUE(mml.find(">a<") != std::string::npos) << "Base 'a' must be present";
    EXPECT_TRUE(mml.find(">1<") != std::string::npos) << "Subscript '1' must be present";
}

TEST(Omml2MathmlTest, SubSup)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:sSubSup><m:e><m:r><m:t>A</m:t></m:r></m:e>"
        "<m:sub><m:r><m:t>1</m:t></m:r></m:sub>"
        "<m:sup><m:r><m:t>2</m:t></m:r></m:sup></m:sSubSup>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<msubsup>") != std::string::npos) << "Must produce msubsup";
    EXPECT_TRUE(mml.find(">A<") != std::string::npos) << "Base 'A' must be present";
    EXPECT_TRUE(mml.find(">1<") != std::string::npos) << "Subscript '1' must be present";
    EXPECT_TRUE(mml.find(">2<") != std::string::npos) << "Superscript '2' must be present";
}

TEST(Omml2MathmlTest, ItalicStyle)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:r><m:rPr><m:sty m:val=\"i\"/></m:rPr><m:t>x</m:t></m:r>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("mathvariant=\"italic\"") != std::string::npos)
        << "Italic style must produce mathvariant=\"italic\"";
    EXPECT_TRUE(mml.find("<mi") != std::string::npos) << "Must produce mi";
}

TEST(Omml2MathmlTest, OverAndUnderLimits)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:limUpp><m:e><m:r><m:t>x</m:t></m:r></m:e>"
        "<m:lim><m:r><m:t>\u00AF</m:t></m:r></m:lim></m:limUpp>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<mover>") != std::string::npos) << "Must produce mover";
    EXPECT_TRUE(mml.find(">x<") != std::string::npos) << "Base 'x' must be present";
}

TEST(Omml2MathmlTest, NarySum)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:nary><m:naryPr><m:chr m:val=\"\u2211\"/>"
        "<m:limLoc m:val=\"undOvr\"/></m:naryPr>"
        "<m:sub><m:r><m:t>i=1</m:t></m:r></m:sub>"
        "<m:sup><m:r><m:t>n</m:t></m:r></m:sup>"
        "<m:e><m:r><m:t>i</m:t></m:r></m:e></m:nary>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<munderover>") != std::string::npos)
        << "Nary with limLoc=undOvr must produce munderover";
    EXPECT_TRUE(mml.find("\u2211") != std::string::npos) << "Sum symbol must be present";
}

TEST(Omml2MathmlTest, Delimiters)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:d><m:dPr><m:begChr m:val=\"[\"/><m:endChr m:val=\"]\"/></m:dPr>"
        "<m:e><m:r><m:t>x</m:t></m:r></m:e></m:d>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find(">[<") != std::string::npos) << "Opening bracket '[' must be present";
    EXPECT_TRUE(mml.find(">]<") != std::string::npos) << "Closing bracket ']' must be present";
}

TEST(Omml2MathmlTest, Accent)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:acc><m:accPr><m:chr m:val=\"\u0302\"/></m:accPr>"
        "<m:e><m:r><m:t>x</m:t></m:r></m:e></m:acc>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<mover>") != std::string::npos) << "Accent must produce mover";
    EXPECT_TRUE(mml.find(">x<") != std::string::npos) << "Base 'x' must be present";
}

TEST(Omml2MathmlTest, FractionNoBar)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:f><m:fPr><m:type m:val=\"noBar\"/></m:fPr>"
        "<m:num><m:r><m:t>a</m:t></m:r></m:num>"
        "<m:den><m:r><m:t>b</m:t></m:r></m:den></m:f>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("linethickness=\"0\"") != std::string::npos)
        << "noBar fraction must have linethickness=\"0\"";
}

TEST(Omml2MathmlTest, DoubleStruckFont)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:r><m:rPr><m:scr m:val=\"double-struck\"/></m:rPr><m:t>R</m:t></m:r>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("mathvariant=\"double-struck\"") != std::string::npos)
        << "double-struck scr must produce mathvariant=\"double-struck\"";
}

TEST(Omml2MathmlTest, NorFlagMtext)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:r><m:rPr><m:nor/></m:rPr><m:t>text</m:t></m:r>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<mtext>") != std::string::npos)
        << "nor flag must produce mtext";
    EXPECT_TRUE(mml.find(">text<") != std::string::npos) << "text content must be present";
}

TEST(Omml2MathmlTest, Table)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:m><m:mr><m:mc><m:r><m:t>a</m:t></m:r></m:mc>"
        "<m:mc><m:r><m:t>b</m:t></m:r></m:mc></m:mr>"
        "<m:mr><m:mc><m:r><m:t>c</m:t></m:r></m:mc>"
        "<m:mc><m:r><m:t>d</m:t></m:r></m:mc></m:mr></m:m>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<mtable>") != std::string::npos) << "Must produce mtable";
    EXPECT_TRUE(mml.find("<mtr>") != std::string::npos) << "Must produce mtr";
    EXPECT_TRUE(mml.find("<mtd>") != std::string::npos) << "Must produce mtd";
    EXPECT_TRUE(mml.find(">a<") != std::string::npos) << "Cell 'a' must be present";
    EXPECT_TRUE(mml.find(">d<") != std::string::npos) << "Cell 'd' must be present";
}

TEST(Omml2MathmlTest, OmathPara)
{
    std::string omml =
        "<m:oMathPara xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:oMath>"
        "<m:r><m:t>x</m:t></m:r>"
        "</m:oMath>"
        "</m:oMathPara>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("display=\"block\"") != std::string::npos)
        << "oMathPara must produce display=\"block\"";
    EXPECT_TRUE(mml.find(">x<") != std::string::npos) << "Content 'x' must be present";
}

TEST(Omml2MathmlTest, Phantom)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:phant><m:r><m:t>x</m:t></m:r></m:phant>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<mphantom>") != std::string::npos) << "Must produce mphantom";
    EXPECT_TRUE(mml.find(">x<") != std::string::npos) << "Content 'x' must be present";
}

TEST(Omml2MathmlTest, FunctionApply)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:func>"
        "<m:fName><m:r><m:t>sin</m:t></m:r></m:fName>"
        "<m:e><m:r><m:t>x</m:t></m:r></m:e>"
        "</m:func>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find(">sin<") != std::string::npos) << "Function name 'sin' must be present";
    EXPECT_TRUE(mml.find(">x<") != std::string::npos) << "Argument 'x' must be present";
}

TEST(Omml2MathmlTest, Prescript)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:sPre>"
        "<m:e><m:r><m:t>X</m:t></m:r></m:e>"
        "<m:sub><m:r><m:t>1</m:t></m:r></m:sub>"
        "<m:sup><m:r><m:t>2</m:t></m:r></m:sup>"
        "</m:sPre>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<mmultiscripts>") != std::string::npos)
        << "Prescript must produce mmultiscripts";
    EXPECT_TRUE(mml.find("<mprescripts/>") != std::string::npos ||
                mml.find("<mprescripts>") != std::string::npos)
        << "Must contain mprescripts";
    EXPECT_TRUE(mml.find(">X<") != std::string::npos) << "Base 'X' must be present";
}

TEST(Omml2MathmlTest, BorderBox)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:borderBox><m:r><m:t>x</m:t></m:r></m:borderBox>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<menclose") != std::string::npos) << "Must produce menclose";
    EXPECT_TRUE(mml.find("notation=\"box\"") != std::string::npos)
        << "Must have notation=\"box\"";
}

TEST(Omml2MathmlTest, NumberDetection)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:r><m:t>42</m:t></m:r>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<mn>42</mn>") != std::string::npos)
        << "All-digit content should become mn";
}

TEST(Omml2MathmlTest, OperatorDetection)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:r><m:t>+</m:t></m:r>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("<mo>+</mo>") != std::string::npos)
        << "Operator character should become mo";
}

TEST(Omml2MathmlTest, BoldStyle)
{
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:r><m:rPr><m:sty m:val=\"b\"/></m:rPr><m:t>x</m:t></m:r>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml).value();

    EXPECT_TRUE(mml.find("mathvariant=\"bold\"") != std::string::npos)
        << "Bold style must produce mathvariant=\"bold\"";
}

// ── New forward mappings (MathML → OMML) ─────────────────────────────────

TEST(Mathml2OmmlTest, Phantom)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><mphantom><mi>x</mi></mphantom></mrow>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml).value();

    EXPECT_TRUE(omml.find("<m:phant>") != std::string::npos) << "mphantom must produce m:phant";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "Content must be preserved";
}

TEST(Mathml2OmmlTest, Prescript)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><mmultiscripts><mi>X</mi><mprescripts/>"
        "<mn>1</mn><mn>2</mn></mmultiscripts></mrow>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml).value();

    EXPECT_TRUE(omml.find("<m:sPre>") != std::string::npos) << "mmultiscripts must produce m:sPre";
    EXPECT_TRUE(omml.find(">X<") != std::string::npos) << "Base 'X' must be present";
    EXPECT_TRUE(omml.find(">1<") != std::string::npos) << "Prescript '1' must be present";
    EXPECT_TRUE(omml.find(">2<") != std::string::npos) << "Presuperscript '2' must be present";
}

TEST(Mathml2OmmlTest, Linebreak)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><mi>x</mi><mspace linebreak=\"newline\"/><mi>y</mi></mrow>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml).value();

    EXPECT_TRUE(omml.find("<m:br/>") != std::string::npos) << "mspace with linebreak must produce m:br";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "x must be present";
    EXPECT_TRUE(omml.find(">y<") != std::string::npos) << "y must be present";
}

TEST(Mathml2OmmlTest, MspaceDropped)
{
    std::string mml =
        "<math><semantics>"
        "<mrow><mi>a</mi><mspace width=\"3pt\"/><mi>b</mi></mrow>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml).value();

    EXPECT_TRUE(omml.find("<m:br/>") == std::string::npos) << "Plain mspace must not produce m:br";
    EXPECT_TRUE(omml.find(">a<") != std::string::npos) << "a must be present";
    EXPECT_TRUE(omml.find(">b<") != std::string::npos) << "b must be present";
    // Just check the mspace is silently removed — a and b should be adjacent
    size_t posA = omml.find(">a<");
    size_t posB = omml.find(">b<");
    EXPECT_LT(posA, posB) << "a must come before b (mspace dropped)";
}

TEST(Mathml2OmmlTest, MstyleMathvariant)
{
    std::string mml =
        "<math><semantics>"
        "<mstyle mathvariant=\"bold\"><mi>x</mi></mstyle>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml).value();

    EXPECT_TRUE(omml.find("<m:sty m:val=\"b\"/>") != std::string::npos)
        << "mstyle mathvariant=bold must propagate m:sty val=b onto child";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "Child content must be preserved";
}

TEST(Mathml2OmmlTest, MpaddedToBox)
{
    std::string mml =
        "<math><semantics>"
        "<mpadded width=\"0\"><mi>x</mi></mpadded>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml).value();

    EXPECT_TRUE(omml.find("<m:box>") != std::string::npos) << "mpadded must produce m:box";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "Child content must be preserved";
}

TEST(Mathml2OmmlTest, MerrorToBox)
{
    std::string mml =
        "<math><semantics>"
        "<merror><mi>x</mi></merror>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml).value();

    EXPECT_TRUE(omml.find("<m:box>") != std::string::npos) << "merror must produce m:box";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "Child content must be preserved";
}

TEST(Mathml2OmmlTest, MencloseToBorderBox)
{
    std::string mml =
        "<math><semantics>"
        "<menclose notation=\"box\"><mi>x</mi></menclose>"
        "</semantics></math>";

    std::string omml = MathmlToOmml::convert(mml).value();

    EXPECT_TRUE(omml.find("<m:borderBox>") != std::string::npos) << "menclose must produce m:borderBox";
    EXPECT_TRUE(omml.find(">x<") != std::string::npos) << "Child content must be preserved";
}

// ── C++23 API: std::expected error signalling, string_view, UTF-8 ──────────

TEST(Mathml2OmmlTest, EmptyInputFails)
{
    auto result = MathmlToOmml::convert("");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "empty input");
}

TEST(Mathml2OmmlTest, NoRootElementFails)
{
    auto result = MathmlToOmml::convert("not xml at all");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "no root element found");
}

TEST(Omml2MathmlTest, EmptyInputFails)
{
    auto result = OmmlToMathml::convert("");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "empty input");
}

TEST(Omml2MathmlTest, NoRootElementFails)
{
    auto result = OmmlToMathml::convert("garbage");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "no root element found");
}

TEST(Mathml2OmmlTest, StringViewInput)
{
    auto result = MathmlToOmml::convert("<math><mi>x</mi></math>");
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("<m:oMath"), std::string::npos);
    EXPECT_NE(result->find(">x<"), std::string::npos);
}

TEST(Mathml2OmmlTest, NumericCharRefDecodedToUtf8)
{
    auto result = MathmlToOmml::convert("<math><mo>&#x2211;</mo><mo>&#960;</mo></math>");
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("\u2211"), std::string::npos) << "Hex ref must decode to UTF-8 sum symbol";
    EXPECT_NE(result->find("\u03C0"), std::string::npos) << "Decimal ref must decode to UTF-8 pi";
}

TEST(Omml2MathmlTest, NumericCharRefDecodedToUtf8)
{
    auto result = OmmlToMathml::convert(
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:r><m:t>&#x3C0;</m:t></m:r>"
        "</m:oMath>");
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("\u03C0"), std::string::npos)
        << "Numeric char ref must decode to UTF-8 in reverse direction too";
}

TEST(Mathml2OmmlTest, SinkOverloadReturnsBool)
{
    struct CountingSink : XmlSink {
        int calls = 0;
        void startElement(std::string_view) override { ++calls; }
        void endElement() override { ++calls; }
        void attribute(std::string_view, std::string_view) override {}
        void characters(std::string_view) override {}
    };

    CountingSink sink;
    EXPECT_TRUE(MathmlToOmml::convert("<math><mi>x</mi></math>", sink));
    EXPECT_GT(sink.calls, 0) << "Sink must be fed elements";
    EXPECT_FALSE(MathmlToOmml::convert("", sink));
}

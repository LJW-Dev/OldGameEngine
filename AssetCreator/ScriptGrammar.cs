using Irony.Parsing;
using System.Xml.Serialization;

namespace AssetCompiler
{
    [Language("Script", "1.0", "Grammar for my Game Engine Script")]
    public class ScriptGrammar : Grammar
    {
        public ScriptGrammar() : base(false) //change case sensitivity to false
        {
            //Comments
            var lineComment = new CommentTerminal("line-comment", "//",
                "\r", "\n", "\u2085", "\u2028", "\u2029");
            NonGrammarTerminals.Add(lineComment);

            var numberLiteral = new NumberLiteral("numberLiteral", NumberOptions.AllowSign);
            var stringLiteral = new StringLiteral("stringLiteral", "\"");
            var identifier = new IdentifierTerminal("identifier", @"_0123456789", "_");

            var opcodeParams = new NonTerminal("opcodeParams");
            var opcode = new NonTerminal("opcode");
            var jumpPos = new NonTerminal("jumpPos");
            var line = new NonTerminal("line");

            var program = new NonTerminal("program");
            Root = program;

            program.Rule = MakeStarRule(program, line);
            line.Rule = opcode | jumpPos;

            opcode.Rule = MakeStarRule(opcode, opcodeParams) + ToTerm(";");
            jumpPos.Rule = identifier + ToTerm(":");

            opcodeParams.Rule = identifier | numberLiteral;
        }
    }
}

using Irony.Parsing;
using System.Xml.Serialization;

namespace AssetCompiler
{
    [Language("Script", "1.0", "Grammar for my Game Engine Script")]
    public class ScriptGrammar : Grammar
    {
        public ScriptGrammar()
        {
            StringLiteral stringLiteral = TerminalFactory.CreateCSharpString("stringLiteral");
            NumberLiteral number = TerminalFactory.CreateCSharpNumber("number");
            IdentifierTerminal identifier = TerminalFactory.CreateCSharpIdentifier("identifier");

            CommentTerminal lineComment = new CommentTerminal("lineComment", "//", "\r", "\n", "\u2085", "\u2028", "\u2029");
            NonGrammarTerminals.Add(lineComment);

            this.MarkPunctuation(";", ",", "(", ")", "{", "}", "=", "wait");

            this.RegisterOperators(9, "+", "-");
            this.RegisterOperators(10, "*", "/");

            var script = new NonTerminal("script");
            this.Root = script;

            var expression = new NonTerminal("expression");
            var bracket = new NonTerminal("bracket");
            var assignment = new NonTerminal("assignment");

            var line = new NonTerminal("line");
            var lineRule = new NonTerminal("lineRule");
            var lineList = new NonTerminal("lineList");

            var binary_operator = new NonTerminal("binary_operator");
            var binary_expression = new NonTerminal("binary_expression");

            var call = new NonTerminal("call");
            var wait = new NonTerminal("wait");
            var paramList = new NonTerminal("paramList");

            var function = new NonTerminal("function");
            var functionFlags = new NonTerminal("functionFlags");
            var functionCode = new NonTerminal("functionCode");
            var functionParams = new NonTerminal("functionParams");
            var functionParamsList = new NonTerminal("functionParamsList");

            lineRule.Rule = call | assignment | wait;
            line.Rule = lineRule + ToTerm(";");
            lineList.Rule = MakeStarRule(lineList, line);

            expression.Rule = identifier | number | stringLiteral | bracket | binary_expression;

            binary_operator.Rule = ToTerm("+") | "-" | "*" | "/";
            binary_expression.Rule = expression + binary_operator + expression;

            wait.Rule = ToTerm("wait") + expression;
            call.Rule = identifier + ToTerm("(") + paramList + ToTerm(")");
            paramList.Rule = MakeStarRule(paramList, ToTerm(","), expression);

            bracket.Rule = ToTerm("(") + expression + ToTerm(")");
            assignment.Rule = identifier + ToTerm("=") + expression;

            functionParamsList.Rule = MakeStarRule(functionParamsList, ToTerm(","), identifier);
            functionParams.Rule = ToTerm("(") + functionParamsList + ToTerm(")");
            functionCode.Rule = ToTerm("{") + lineList + ToTerm("}");
            functionFlags.Rule = ToTerm("main") + identifier | identifier;
            function.Rule = functionFlags + functionParams + functionCode;

            script.Rule = MakeStarRule(script, function);

            // Mark NonTerminals that aren't needed as ChildNodes
            MarkTransient(lineRule, line, functionCode, functionParams, expression, binary_operator);
        }
    }
}
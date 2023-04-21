using Irony.Parsing;

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

            MarkPunctuation(";", ",", "(", ")", "{", "}", "wait", "return", "if", "else", "do", "while", "for", "thread");

            RegisterOperators(10, "*", "/", "%");
            RegisterOperators(9, "+", "-");
            RegisterOperators(8, "<<", ">>");
            RegisterOperators(7, ">", "<", "<=", ">=");
            RegisterOperators(6, "==", "!=");
            RegisterOperators(5, "&");
            RegisterOperators(4, "^");
            RegisterOperators(3, "|");

            // Conditional Operators
            RegisterOperators(2, "&&");
            RegisterOperators(1, "||");



            var script = new NonTerminal("script");
            Root = script;

            var expression = new NonTerminal("expression");

            var curlyBrackets = new NonTerminal("curlyBrackets");
            var brackets = new NonTerminal("brackets");

            var assignment = new NonTerminal("assignment");
            var assignmentOperator = new NonTerminal("assignmentOperator");

            var line = new NonTerminal("line");
            var lineList = new NonTerminal("lineList");
            var lineStatement = new NonTerminal("lineStatement");
            var lineNoPunctuation = new NonTerminal("lineNoPunctuation");

            var logicalOperator = new NonTerminal("logicalOperator");
            var binaryOperator = new NonTerminal("binaryOperator");
            var binaryExpression = new NonTerminal("binaryExpression");

            var call = new NonTerminal("call");
            var threadCall = new NonTerminal("threadCall");
            var paramList = new NonTerminal("paramList");

            var wait = new NonTerminal("wait");
            var returnStatement = new NonTerminal("returnStatement");
            var jumpStatement = new NonTerminal("jumpStatement");

            var function = new NonTerminal("function");
            var functionFlags = new NonTerminal("functionFlags");
            var functionParams = new NonTerminal("functionParams");
            var functionParamsList = new NonTerminal("functionParamsList");

            var ifStatement = new NonTerminal("ifStatement");

            var whileStatement = new NonTerminal("whileStatement");

            var doStatement = new NonTerminal("doStatement");

            var forStatement = new NonTerminal("forStatement");

            lineStatement.Rule = call | threadCall | assignment | wait | returnStatement | jumpStatement | doStatement;
            lineNoPunctuation.Rule = curlyBrackets | ifStatement | whileStatement | forStatement;
            line.Rule = lineNoPunctuation | lineStatement + ToTerm(";");
            lineList.Rule = MakeStarRule(lineList, line);

            expression.Rule = identifier | number | stringLiteral | brackets | binaryExpression | call;

            binaryOperator.Rule = 
                ToTerm("==") | "!=" 
                | ">" | "<" | "<=" | ">=" 
                | "+" | "-" 
                | "*" | "/" | "%"
                | "|" | "&" | "^" 
                | ">>" | "<<"
                | ToTerm("||") | "&&";
            
            binaryExpression.Rule = expression + binaryOperator + expression;

            wait.Rule = ToTerm("wait") + expression;
            returnStatement.Rule = ToTerm("return") | ToTerm("return") + expression;
            jumpStatement.Rule = ToTerm("break") | ToTerm("continue");

            call.Rule = identifier + ToTerm("(") + paramList + ToTerm(")");
            threadCall.Rule = ToTerm("thread") + identifier + ToTerm("(") + paramList + ToTerm(")");
            paramList.Rule = MakeStarRule(paramList, ToTerm(","), expression);

            brackets.Rule = ToTerm("(") + expression + ToTerm(")");
            curlyBrackets.Rule = ToTerm("{") + lineList + ToTerm("}");

            assignmentOperator.Rule = ToTerm("=") | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>=";
            assignment.Rule = identifier + assignmentOperator + expression;

            functionParamsList.Rule = MakeStarRule(functionParamsList, ToTerm(","), identifier);
            functionParams.Rule = ToTerm("(") + functionParamsList + ToTerm(")");
            functionFlags.Rule = ToTerm("main").Q(); // can have 0 or 1 times
            function.Rule = functionFlags + identifier + functionParams + curlyBrackets;

            ifStatement.Rule = ToTerm("if") + ToTerm("(") + binaryExpression + ToTerm(")") + line 
                | ToTerm("if") + ToTerm("(") + binaryExpression + ToTerm(")") + line + ToTerm("else") + line;

            whileStatement.Rule = ToTerm("while") + ToTerm("(") + binaryExpression + ToTerm(")") + line;

            doStatement.Rule = ToTerm("do") + line + ToTerm("while") + ToTerm("(") + binaryExpression + ToTerm(")");

            forStatement.Rule = ToTerm("for") + ToTerm("(") + assignment + ";" + binaryExpression + ";" + assignment + ToTerm(")") + line;

            script.Rule = MakeStarRule(script, function);

            // Mark NonTerminals that aren't needed as ChildNodes
            MarkTransient(lineStatement, lineNoPunctuation, line, functionParams, functionFlags, expression, logicalOperator, binaryOperator, assignmentOperator);
        }
    }
}
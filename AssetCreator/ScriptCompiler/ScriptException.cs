using System;
using System.Xml.Linq;
using Irony.Parsing;

namespace AssetCompiler
{
    public class ScriptException : Exception
    {
        public ScriptException(string message) : base(message)
        {
            Console.WriteLine("Script Exception: {0}", message);
        }

        public ScriptException(string message, ParseTreeNode node) : base(message)
        {
            Console.WriteLine("Script Exception at line {0}: {1}", node.FindToken().Location.Line + 1, message); // +1 as line starts at 0
        }
    }
}

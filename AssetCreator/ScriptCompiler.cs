using Irony.Parsing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Emit;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace AssetCompiler
{
    public class ScriptCompiler
    {
        List<byte> CompiledScript = new List<byte>();

        Dictionary<string, int> jumpPosDict = new Dictionary<string, int>();

        void emitOpcode(string opcode)
        {
            if (OpcodeList.Contains(opcode))
            {
                CompiledScript.Add((byte)OpcodeList.IndexOf(opcode));
            }
            else
            {
                Console.WriteLine("Not an opcode!");
            }
        }

        void emitInteger(int value)
        {
            CompiledScript.AddRange(BitConverter.GetBytes(value).ToArray());
        }

        void emitFloat(float value)
        {
            CompiledScript.AddRange(BitConverter.GetBytes(value).ToArray());
        }

        void parseOpcode(ParseTreeNode opcodeParams)
        {
            string opcodeName = opcodeParams.ChildNodes[0].ChildNodes[0].Token.ValueString.ToLower();

            emitOpcode(opcodeName);

            switch (opcodeName)
            {
                case "op_pushinteger":
                    emitInteger(int.Parse(opcodeParams.ChildNodes[1].ChildNodes[0].Token.ValueString));
                    break;

                case "op_pushfloat":
                    emitFloat(float.Parse(opcodeParams.ChildNodes[1].ChildNodes[0].Token.ValueString));
                    break;

                case "op_jump":
                    int jumpVal = -1;
                    if (jumpPosDict.TryGetValue(opcodeParams.ChildNodes[1].ChildNodes[0].Token.ValueString, out jumpVal))
                    {
                        emitInteger(jumpVal - CompiledScript.Count);
                    }
                    else
                    {
                        Console.WriteLine("Not a jump pos!");
                    }
                    break;

                case "op_call":
                    string name = opcodeParams.ChildNodes[1].ChildNodes[0].Token.ValueString;
                    if (BuiltinList.Contains(name))
                    {
                        emitInteger(BuiltinList.IndexOf(name));
                    }
                    else
                    {
                        Console.WriteLine("Not a builtin func!");
                    }
                    break;

                case "op_wait":
                    emitFloat(float.Parse(opcodeParams.ChildNodes[1].ChildNodes[0].Token.ValueString));
                    break;

                case "op_end":
                    break;

                default:
                    Console.WriteLine("unknown opcode!");
                    break;
            }
        }

        void addJumpPos(ParseTreeNode jumpPos)
        {
            jumpPosDict.Add(jumpPos.ChildNodes[0].Token.ValueString, CompiledScript.Count);
        }

        public bool CompileScript(string filePath, string outPath)
        {
            var gameScript = new ScriptGrammar();
            var parser = new Parser(gameScript);
            var tree = parser.Parse(File.ReadAllText(filePath));

            if (tree.ParserMessages.Count > 0)
            {
                int errorLoc = tree.ParserMessages[0].Location.Line + 1;
                Console.WriteLine("ERROR: Bad syntax in line " + errorLoc + "."); //when it finds an error the parsing stops, so multiple syntax errors can't be outputted
                return false;
            }

            foreach (ParseTreeNode node in tree.Root.ChildNodes)
            {
                switch(node.ChildNodes[0].Term.Name)
                {
                    case "opcode":
                        parseOpcode(node.ChildNodes[0].ChildNodes[0]);
                        break;

                    case "jumpPos":
                        addJumpPos(node.ChildNodes[0]);
                        break;
                }
                
            }

            File.WriteAllBytes(outPath, CompiledScript.ToArray());

            return true;
        }

        List<string> OpcodeList = new List<string>{
            "op_pushinteger",
            "op_pushfloat",
            "op_jump",
            "op_call",
            "op_wait",
            "op_end"
        };

        List<string> BuiltinList = new List<string>{
            "spawnCube"
        };
    }
}

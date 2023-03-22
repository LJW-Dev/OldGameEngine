using Irony.Parsing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Reflection.Emit;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace AssetCompiler
{
    public class ScriptCompiler
    {
        private scriptInfo script = new scriptInfo();

        uint hashString(string value) // djb2 hashing algorithm http://www.cse.yorku.ca/~oz/hash.html
        {
            ulong hash = 5381;

            foreach (char c in value)
                hash = ((hash << 5) + hash) + c;

            return (uint)(hash & 0xFFFFFFFF);
        }

        void emitOpcode(OpcodeList opcode)
        {
            script.emitByte((byte)opcode);
        }

        void emitGetVariable(string name)
        {
            emitOpcode(OpcodeList.op_getVar);
            script.emitInt(script.currFunc.getLocalVarIndex(name));
        }

        void emitSetVariable(string name)
        {
            emitOpcode(OpcodeList.op_setVar);
            script.emitInt(script.currFunc.getLocalVarIndex(name));
        }

        void emitNumber(ParseTreeNode node)
        {
            switch(node.Token.Value)
            {
                case System.Int32:
                    emitOpcode(OpcodeList.op_pushInt);
                    script.emitInt(Int32.Parse(node.Token.ValueString));
                    break;

                case System.Double:
                case System.Single:
                    emitOpcode(OpcodeList.op_pushFloat);
                    script.emitFloat(Single.Parse(node.Token.ValueString));
                    break;

                default:
                    throw new Exception(String.Format("Unknown type {0}", node.Token.Value.GetType()));
            }
        }

        void emitSetVar(ParseTreeNode node)
        {
            parseExpression(node.ChildNodes[1]);
            emitSetVariable(node.ChildNodes[0].Token.ValueString);
        }

        int parseParamList(ParseTreeNode node)
        {
            int paramCount = 0;

            foreach (ParseTreeNode expr in node.ChildNodes)
            {
                parseExpression(expr);
                paramCount++;
            }

            return paramCount;
        }

        void emitJump(int jumpPos)
        {
            emitOpcode(OpcodeList.op_jump);
            script.emitInt(jumpPos - script.getScriptPos());
        }

        bool isCallBuiltin(string name)
        {
            if (BuiltinList.Contains(name))
                return true;
            else
                return false;
        }

        bool isCallLocal(string name)
        {
            if (script.doesFunctionExist(name))
                return true;
            else
                return false;
        }

        bool emitBuiltinCall(string name, int paramCount)
        {
            if (!isCallBuiltin(name))
                return false;

            emitOpcode(OpcodeList.op_callBuiltin);
            script.emitUInt(hashString(name));

            return true;
        }

        bool emitLocalCall(string name, int paramCount)
        {
            if (!isCallLocal(name))
                return false;

            emitOpcode(OpcodeList.op_callScript);
            script.emitInt(script.getFunction(name).bytecodeStart - script.getScriptPos());

            return true;
        }

        void emitCall(ParseTreeNode node)
        {
            //TODO: check param count matches

            string callName = node.ChildNodes[0].Token.ValueString;
            int paramCount = parseParamList(node.ChildNodes[1]);
            
            if(!emitBuiltinCall(callName, paramCount) && !emitLocalCall(callName, paramCount))
                throw new Exception(String.Format("Function {0} does not exist!", callName));
        }

        void emitWait(ParseTreeNode node)
        {
            parseExpression(node.ChildNodes[0]);
            emitOpcode(OpcodeList.op_wait);
        }

        void emitStringLiteral(ParseTreeNode node)
        {
            emitOpcode(OpcodeList.op_pushString);
            script.emitString(node.Token.ValueString);
        }

        void emitBinaryExpression(ParseTreeNode node)
        {
            foreach(ParseTreeNode expr in node.ChildNodes)
            {
                if(expr.Term.Name.Equals("binary_expression"))
                {
                    emitBinaryExpression(expr);
                }
                else
                {
                    parseExpression(expr);
                }
            }

            switch(node.ChildNodes[1].Term.Name)
            {
                case "+":
                    emitOpcode(OpcodeList.op_add);
                    break;

                case "-":
                    emitOpcode(OpcodeList.op_minus);
                    break;

                case "*":
                    emitOpcode(OpcodeList.op_multiply);
                    break;

                case "/":
                    emitOpcode(OpcodeList.op_divide);
                    break;
            }
        }

        void parseExpression(ParseTreeNode node)
        {
            switch (node.Term.Name)
            {
                case "identifier":
                    emitGetVariable(node.Token.ValueString);
                    break;

                case "number":
                    emitNumber(node);
                    break;

                case "stringLiteral":
                    emitStringLiteral(node);
                    break;

                case "bracket":
                    parseExpression(node.ChildNodes[0]);
                    break;

                case "binary_expression":
                    emitBinaryExpression(node);
                    break;
            }
        }

        void parseFunctionNode(ParseTreeNode node)
        {
            bool shouldAutoExecute;
            string funcName;
            ParseTreeNode funcFlags = node.ChildNodes[0];
            if (funcFlags.ChildNodes[0].Term.Name.Equals("main"))
            {
                shouldAutoExecute = true;
                funcName = funcFlags.ChildNodes[1].Token.ValueString;
            }
            else
            {
                shouldAutoExecute = false;
                funcName = funcFlags.ChildNodes[0].Token.ValueString;
            }
            script.addNewFunction(funcName, shouldAutoExecute);

            ParseTreeNode funcParamsList = node.ChildNodes[1];
            foreach (ParseTreeNode param in funcParamsList.ChildNodes)
            {
                script.currFunc.addLocalVar(param.Token.ValueString);
            }

            if(funcParamsList.ChildNodes.Count > 0) 
            {
                if (shouldAutoExecute)
                    throw new Exception("Main function cannot have parameters.");

                emitOpcode(OpcodeList.op_popParams);
                script.emitByte((byte)funcParamsList.ChildNodes.Count);
            }

            ParseTreeNode codeLineList = node.ChildNodes[2];
            foreach (ParseTreeNode line in codeLineList.ChildNodes)
            {
                switch(line.Term.Name)
                {
                    case "call":
                        emitCall(line);
                        break;

                    case "assignment":
                        emitSetVar(line);
                        break;

                    case "wait":
                        emitWait(line);
                        break;
                }
            }

            emitOpcode(OpcodeList.op_end);
        }

        public bool CompileScript(string filePath, string outPath)
        {
            var gameScript = new ScriptGrammar();
            var parser = new Parser(gameScript);
            var tree = parser.Parse(File.ReadAllText(filePath));

            if (tree.ParserMessages.Count > 0)
            {
                int errorLoc = tree.ParserMessages[0].Location.Line + 1;
                Console.WriteLine("ERROR: Bad syntax in line " + errorLoc);
                return false;
            }

            foreach (ParseTreeNode node in tree.Root.ChildNodes)
            {
                parseFunctionNode(node);
            }
        
            script.writeHeader();

            File.WriteAllBytes(outPath, script.getCompiledScript());

            return true;
        }

        enum OpcodeList
        {
            op_pushInt,
            op_pushFloat,
            op_jump,
            op_callBuiltin,
            op_callScript,
            op_wait,
            op_getVar,
            op_setVar,
            op_add,
            op_minus,
            op_multiply,
            op_divide,
            op_pushString,
            op_popParams,
            op_end
        };

        List<string> BuiltinList = new List<string>{
            "spawnCube",
            "debugPrint"
        };

        private class scriptInfo
        {
            private List<byte> compiledScript = new List<byte>();
            private Dictionary<string, functionInfo> functionDict = new Dictionary<string, functionInfo>(); // Key: name

            public functionInfo currFunc;
            private string mainFunction = String.Empty;

            public scriptInfo()
            {
                // Init header, will be overwritten later
                emitInt(0); // script size
                emitInt(0); // auto exec pos
            }

            public int getScriptPos()
            {
                return compiledScript.Count;
            }

            public void replaceBytes(int pos, byte[] value)
            {
                compiledScript.RemoveRange(pos, value.Length);
                compiledScript.InsertRange(pos, value);
            }

            public void emitByte(byte val)
            {
                compiledScript.Add(val);
            }

            public void emitInt(int val)
            {
                compiledScript.AddRange(BitConverter.GetBytes(val));
            }

            public void emitUInt(uint val)
            {
                compiledScript.AddRange(BitConverter.GetBytes(val));
            }

            public void emitFloat(float val)
            {
                compiledScript.AddRange(BitConverter.GetBytes(val));
            }

            public void emitString(string val)
            {
                compiledScript.AddRange(Encoding.ASCII.GetBytes(val + '\0'));
            }

            private bool doesHaveMainFunction()
            {
                return mainFunction != String.Empty;
            }

            public functionInfo getFunction(string name)
            {
                return functionDict[name];
            }

            public bool doesFunctionExist(string name)
            {
                return functionDict.ContainsKey(name);
            }

            public void addNewFunction(string name, bool autoExecute)
            {
                if (autoExecute)
                {
                    if (doesHaveMainFunction())
                        throw new Exception("More than one main function!");

                    mainFunction = name;
                }

                functionDict.Add(name, new functionInfo(getScriptPos()));
                currFunc = functionDict[name];
            }

            public byte[] getCompiledScript()
            {
                return compiledScript.ToArray();
            }

            private readonly int HEADER_SIZE = 8;

            public void writeHeader()
            {
                if (!doesHaveMainFunction())
                    throw new Exception("Script must have at least one main function!");

                replaceBytes(0, BitConverter.GetBytes(getScriptPos() - HEADER_SIZE));
                replaceBytes(4, BitConverter.GetBytes(getFunction(mainFunction).bytecodeStart - HEADER_SIZE));
            }
        }

        private class functionInfo
        {
            private List<string> localVarList = new List<string>();

            public int bytecodeStart = -1;

            private readonly int SCR_MAX_VARS = 20;

            public functionInfo(int bytecodeStart)
            {
                this.bytecodeStart = bytecodeStart;
            }

            public int addLocalVar(string name)
            {
                if(localVarList.Count == SCR_MAX_VARS)
                    throw new Exception("More than SCR_MAX_VARS local variables!");

                localVarList.Add(name);
                return localVarList.IndexOf(name);
            }

            public int getLocalVarIndex(string name)
            {
                int varIndex = localVarList.IndexOf(name);
                if (varIndex == -1)
                    return addLocalVar(name);
                else
                    return varIndex;
            }
        }
    }
}
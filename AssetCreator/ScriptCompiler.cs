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

        void emitBuiltinCall(string name, int paramCount)
        {
            emitOpcode(OpcodeList.op_callBuiltin);
            script.emitInt(BuiltinList.IndexOf(name));
        }

        void emitLocalCall(string name, int paramCount)
        {
            emitOpcode(OpcodeList.op_callScript);
            script.emitByte((byte)paramCount);
            script.emitInt(script.getFunction(name).bytecodeStart - script.getScriptPos());
        }

        void emitCall(ParseTreeNode node)
        {
            int paramCount = parseParamList(node.ChildNodes[1]);

            string callName = node.ChildNodes[0].Token.ValueString;
            if (isCallBuiltin(callName))
                emitBuiltinCall(callName, paramCount);
            else if (isCallLocal(callName))
                emitLocalCall(callName, paramCount);
            else
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
                Console.WriteLine("ERROR: Bad syntax in line " + errorLoc + ".");
                return false;
            }

            int autoExecFuncPos = script.getScriptPos();
            script.emitInt(0); // will be overwritten later

            foreach (ParseTreeNode node in tree.Root.ChildNodes)
            {
                parseFunctionNode(node);
            }

            if (script.doesFunctionExist(script.autoExecuteFunction))
            {
                int byteCodeStart = script.getFunction(script.autoExecuteFunction).bytecodeStart;
                script.replaceBytes(autoExecFuncPos, BitConverter.GetBytes(byteCodeStart));
            }
                
            else
                throw new Exception("Script must have at least one main function!");
            

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
            public string autoExecuteFunction = "";

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

            public void emitFloat(float val)
            {
                compiledScript.AddRange(BitConverter.GetBytes(val));
            }

            public void emitString(string val)
            {
                compiledScript.AddRange(Encoding.ASCII.GetBytes(val + '\0'));
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
                    if (autoExecuteFunction.Equals(""))
                        autoExecuteFunction = name;
                    else
                        throw new Exception("More than one main function!");

                }

                functionDict.Add(name, new functionInfo(getScriptPos()));
                currFunc = functionDict[name];
            }

            public byte[] getCompiledScript()
            {
                return compiledScript.ToArray();
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
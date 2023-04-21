using Irony.Parsing;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection.PortableExecutable;
using System.Text;
using System.Xml.Linq;
using static AssetCompiler.ScriptDef;

namespace AssetCompiler
{
    public class ScriptCompiler
    {
        private scriptInfo script;
        private List<loopInfo> loopList;
        private logicalOperatorsInfo logicalOperators;

        public ScriptCompiler()
        {
            script = new scriptInfo();
            loopList = new List<loopInfo>();
            logicalOperators = new logicalOperatorsInfo();
        }
        
        void emitGetVariable(string name)
        {
            script.emitOpcode(OpcodeList.op_getVar, script.currFunc().getLocalVarIndex(name));
        }

        void emitNumber(ParseTreeNode node)
        {
            switch(node.Token.Value)
            {
                case System.Int32:
                    script.emitOpcode(OpcodeList.op_pushInt, Int32.Parse(node.Token.ValueString));
                    break;

                case System.Double:
                case System.Single:
                    script.emitOpcode(OpcodeList.op_pushFloat, Single.Parse(node.Token.ValueString));
                    break;

                default:
                    throw new Exception(String.Format("Unknown type {0}", node.Token.Value.GetType()));
            }
        }

        void emitSetVar(ParseTreeNode node)
        {
            string varName = node.ChildNodes[0].Token.ValueString;
            string operatorType = node.ChildNodes[1].Token.ValueString;

            if (operatorType == "=")
            {
                parseExpression(node.ChildNodes[2]);
                script.emitOpcode(OpcodeList.op_setVar, script.currFunc().getLocalVarIndex(varName));
                return;
            }

            // Parse backwards as math opcodes use first operand as the first on the stack
            parseExpression(node.ChildNodes[2]);
            emitIdentifier(node.ChildNodes[0], true);

            switch (operatorType)
            {
                case "+=":
                    script.emitOpcode(OpcodeList.op_add);
                    break;

                case "-=":
                    script.emitOpcode(OpcodeList.op_minus);
                    break;

                case "*=":
                    script.emitOpcode(OpcodeList.op_multiply);
                    break;

                case "/=":
                    script.emitOpcode(OpcodeList.op_divide);
                    break;

                case "%=":
                    script.emitOpcode(OpcodeList.op_modulus);
                    break;

                case "&=":
                    script.emitOpcode(OpcodeList.op_and);
                    break;

                case "|=":
                    script.emitOpcode(OpcodeList.op_or);
                    break;

                case "^=":
                    script.emitOpcode(OpcodeList.op_xor);
                    break;

                case "<<=":
                    script.emitOpcode(OpcodeList.op_shiftLeft);
                    break;

                case ">>=":
                    script.emitOpcode(OpcodeList.op_shiftRight);
                    break;

                default:
                    throw new Exception(String.Format("Unknown assignment operator {0}", operatorType));
            }
            
            script.emitOpcode(OpcodeList.op_setVar, script.currFunc().getLocalVarIndex(varName));
        }

        int parseParamList(ParseTreeNode node)
        {
            int paramCount = 0;

            // Reverse params so first param is at the top of the stack
            node.ChildNodes.Reverse();

            foreach (ParseTreeNode expr in node.ChildNodes)
            {
                parseExpression(expr);
                paramCount++;
            }

            return paramCount;
        }

        // Insert jump opcode
        void emitJump(int jumpToHere)
        {
            int jumpDistance = jumpToHere - (script.getPos() + 1); // +1 to account for opcode

            script.emitOpcode(OpcodeList.op_jump, jumpDistance);
        }

        void emitBooleanJump(int jumpToHere, bool isJumpOnTrue)
        {
            int jumpDistance = jumpToHere - (script.getPos() + 1); // +1 to account for opcode

            if(isJumpOnTrue)
                script.emitOpcode(OpcodeList.op_jumpOnTrue, jumpDistance);
            else
                script.emitOpcode(OpcodeList.op_jumpOnFalse, jumpDistance);
        }
        
        // Overwrite a jump opcode's value
        void insertJump(int jumpFromHere, int jumpToHere)
        {
            int jumpDistance = jumpToHere - jumpFromHere;

            script.replaceBytes(jumpFromHere, BitConverter.GetBytes(jumpDistance));
        }

        bool emitBuiltinCall(string name, int paramCount, bool isThreaded, ParseTreeNode node)
        {
            if(!BuiltinList.ContainsKey(name))
                return false;

            int builtinParamCount = BuiltinList[name];

            if (builtinParamCount != paramCount)
                throw new ScriptException(String.Format("Cannot call {0} with {1} parameters, expected {2}", name, paramCount, builtinParamCount), node);
            
            if (isThreaded)
                throw new ScriptException(String.Format("Cannot thread builtin {0}.", name), node);

            int instructionPtr = script.emitOpcode(OpcodeList.op_callBuiltin, script.dummyLongVal);

            script.addImport(name, instructionPtr);

            return true;
        }

        bool emitLocalCall(string name, int paramCount, bool isThreaded, ParseTreeNode node)
        {
            functionInfo? function = script.getFunction(name);

            if (function == null)
                return false;

            if (function.info.paramCount != paramCount)
                throw new ScriptException(String.Format("Cannot call {0} with {1} parameters when {2} is required", name, paramCount, function.info.paramCount), node);

            OpcodeList opcode;
            if (isThreaded)
                opcode = OpcodeList.op_callScriptThreaded;
            else
                opcode = OpcodeList.op_callScript;

            script.emitOpcode(opcode, (byte)paramCount, function.info.bytecodePtr - (script.getPos() + 1)); // +1 to account for opcode

            return true;
        }

        void emitCall(ParseTreeNode node, bool isThreaded)
        {
            string callName = node.ChildNodes[0].Token.ValueString;
            int paramCount = parseParamList(node.ChildNodes[1]);

            if(paramCount > byte.MaxValue)
                throw new ScriptException(String.Format("Function {0} cannot have more than 255 parameters! (wow)", callName), node);

            if (!emitLocalCall(callName, paramCount, isThreaded, node) && !emitBuiltinCall(callName, paramCount, isThreaded, node))
                throw new ScriptException(String.Format("Function {0} does not exist!", callName), node);
        }

        void emitWait(ParseTreeNode node)
        {
            parseExpression(node.ChildNodes[0]);
            script.emitOpcode(OpcodeList.op_wait);
        }

        void emitStringLiteral(ParseTreeNode node)
        {
            int instructionPtr = script.emitOpcode(OpcodeList.op_pushString, script.dummyLongVal);
            script.addString(node.Token.ValueString, instructionPtr);
        }

        void emitMathExpression(ParseTreeNode node)
        {
            // parse right node first as operators use the top of the stack as the first operand, next on the stack as second
            parseExpression(node.ChildNodes[2]);
            parseExpression(node.ChildNodes[0]);

            switch (node.ChildNodes[1].Token.ValueString)
            {
                case "+":
                    script.emitOpcode(OpcodeList.op_add);
                    break;

                case "-":
                    script.emitOpcode(OpcodeList.op_minus);
                    break;

                case "*":
                    script.emitOpcode(OpcodeList.op_multiply);
                    break;

                case "/":
                    script.emitOpcode(OpcodeList.op_divide);
                    break;

                case "%":
                    script.emitOpcode(OpcodeList.op_modulus);
                    break;

                case "|":
                    script.emitOpcode(OpcodeList.op_or);
                    break;

                case "&":
                    script.emitOpcode(OpcodeList.op_and);
                    break;

                case "^":
                    script.emitOpcode(OpcodeList.op_xor);
                    break;

                case "<<":
                    script.emitOpcode(OpcodeList.op_shiftLeft);
                    break;

                case ">>":
                    script.emitOpcode(OpcodeList.op_shiftRight);
                    break;

                case "==":
                    script.emitOpcode(OpcodeList.op_equal);
                    break;

                case "!=":
                    script.emitOpcode(OpcodeList.op_notEqual);
                    break;

                case ">":
                    script.emitOpcode(OpcodeList.op_greaterThan);
                    break;

                case "<":
                    script.emitOpcode(OpcodeList.op_lessThan);
                    break;

                case ">=":
                    script.emitOpcode(OpcodeList.op_greaterThanOrEqual);
                    break;

                case "<=":
                    script.emitOpcode(OpcodeList.op_lessThanOrEqual);
                    break;

                default:
                    throw new Exception(String.Format("Unknown binary expression operator {0}", node.ChildNodes[1].Token.ValueString));
            }
        }

        void emitLogicalOperatorORJumps(int trueResultPos)
        {
            foreach (int jump in logicalOperators.popORs())
            {
                insertJump(jump, trueResultPos);
            }
        }

        void emitLogicalOperatorANDJumps(int falseResultPos)
        {
            foreach(int jump in logicalOperators.popANDs())
            {
                insertJump(jump, falseResultPos);
            }
        }

        // Call after parsing an expression that might have logical operators used
        void emitLogicalOperatorJumps(int trueResultPos, int falseResultPos)
        {
            emitLogicalOperatorORJumps(trueResultPos);
            emitLogicalOperatorANDJumps(falseResultPos);
        }

        void emitTempLogicalOperatorJump(string operatorName)
        {
            if (operatorName == "||")
            {
                logicalOperators.pushOR(script.emitOpcode(OpcodeList.op_jumpOnTrue, script.tempJumpVal));
            }
            else if (operatorName == "&&")
            {
                logicalOperators.pushAND(script.emitOpcode(OpcodeList.op_jumpOnFalse, script.tempJumpVal));
            }
            
            int currPos = script.getPos();
            emitLogicalOperatorJumps(currPos, currPos);
        }

        void emitLogicalExpression(ParseTreeNode node)
        {
            logicalOperators.operatorCount++;

            parseExpression(node.ChildNodes[0]);

            emitTempLogicalOperatorJump(node.ChildNodes[1].Token.ValueString);
            
            parseExpression(node.ChildNodes[2]);

            // No adding jumps here, as we let the result of the second expression be checked by whatever called emitBinaryExpression first

            logicalOperators.operatorCount--;
        }

        void emitBinaryExpression(ParseTreeNode node)
        {
            // Irony auto sorts the operators based on prececence 

            string operatorType = node.ChildNodes[1].Term.Name;

            if (operatorType == "||" || operatorType == "&&")
                emitLogicalExpression(node);
            else
                emitMathExpression(node);
        }

        void emitReturn(ParseTreeNode node)
        {
            if(node.ChildNodes.Count != 0)
                parseExpression(node.ChildNodes[0]);

            script.emitOpcode(OpcodeList.op_end);
        }
        
        int startLoop()
        {
            int loopStart = script.getPos();

            loopList.Add(new loopInfo(loopStart));

            return loopStart;
        }

        void endLoop()
        {
            if(loopList.Count == 0)
                throw new Exception("addLoopJump: loopList is empty.");
            
            loopInfo loop = loopList.Last();
          
            foreach(int breakPos in loop.breakList)
            {
                insertJump(breakPos, script.getPos());
            }

            foreach (int continuePos in loop.continueList)
            {
                insertJump(continuePos, loop.loopStart);
            }

            loopList.Remove(loop);
        }

        void addLoopJump(bool isBreak)
        {
            if (loopList.Count == 0)
                throw new Exception("addLoopJump: loopList is empty.");

            loopInfo loop = loopList.Last();
            int jumpPos = script.emitOpcode(OpcodeList.op_jump, script.tempJumpVal);

            if (isBreak)
                loop.breakList.Add(jumpPos);
            else
                loop.continueList.Add(jumpPos);
        }

        void emitBreakOrContinue(ParseTreeNode node)
        {
            addLoopJump(node.ChildNodes[0].Token.ValueString.Equals("break"));
        }

        void emitWhileStatement(ParseTreeNode node)
        {
            int loopStart = startLoop();

            emitBinaryExpression(node.ChildNodes[0]);
            int jumpToEnd = script.emitOpcode(OpcodeList.op_jumpOnFalse, script.tempJumpVal);

            int trueJumpPos = script.getPos();
            parseLine(node.ChildNodes[1]);

            emitJump(loopStart);

            int endLoopPos = script.getPos();
            emitLogicalOperatorJumps(trueJumpPos, endLoopPos);
            insertJump(jumpToEnd, endLoopPos);
            
            endLoop();
        }

        void emitDoStatement(ParseTreeNode node)
        {
            int loopStart = startLoop();

            parseLine(node.ChildNodes[0]);

            emitBinaryExpression(node.ChildNodes[1]);

            emitBooleanJump(loopStart, true);

            emitLogicalOperatorJumps(loopStart, script.getPos());

            endLoop();
        }
        
        void emitIfStatement(ParseTreeNode node)
        {
            emitBinaryExpression(node.ChildNodes[0]);
            int jumpPastTrueResult = script.emitOpcode(OpcodeList.op_jumpOnFalse, script.tempJumpVal);

            int trueJumpPos = script.getPos();
            parseLine(node.ChildNodes[1]);

            if (node.ChildNodes.Count == 3) // if else statement
            {
                int jumpPastElseStatement = script.emitOpcode(OpcodeList.op_jump, script.tempJumpVal);

                int falseJumpPos = script.getPos();
                emitLogicalOperatorJumps(trueJumpPos, falseJumpPos);
                insertJump(jumpPastTrueResult, falseJumpPos);

                parseLine(node.ChildNodes[2]);

                insertJump(jumpPastElseStatement, script.getPos());
            }
            else // if statement only
            {
                int falseJumpPos = script.getPos();
                emitLogicalOperatorJumps(trueJumpPos, falseJumpPos);
                insertJump(jumpPastTrueResult, falseJumpPos);
            }
        }

        void emitForStatement(ParseTreeNode node)
        {
            emitSetVar(node.ChildNodes[0]);
            int loopStart = startLoop();
            
            emitBinaryExpression(node.ChildNodes[1]);
            int jumpToEnd = script.emitOpcode(OpcodeList.op_jumpOnFalse, script.tempJumpVal);

            int trueJumpPos = script.getPos();
            parseLine(node.ChildNodes[3]);
            emitSetVar(node.ChildNodes[2]);

            emitJump(loopStart);

            int endLoopPos = script.getPos();
            emitLogicalOperatorJumps(trueJumpPos, endLoopPos);
            insertJump(jumpToEnd, endLoopPos);

            endLoop();
        }

        void emitIdentifier(ParseTreeNode node, bool hasToBeVariable)
        {
            string identifier = node.Token.ValueString;

            switch(identifier)
            {
                case "true":
                    script.emitOpcode(OpcodeList.op_pushBool, (byte)1);
                    break;

                case "false":
                    script.emitOpcode(OpcodeList.op_pushBool, (byte)0);
                    break;

                case "null":
                    script.emitOpcode(OpcodeList.op_pushNull);
                    break;

                default:
                    emitGetVariable(identifier);
                    return; // skip hasToBeVariable check
            }

            if (hasToBeVariable)
                throw new ScriptException("emitIdentifier: identifier has to be a variable.", node);
        }
        
        void parseBrackets(ParseTreeNode node)
        {
            parseExpression(node.ChildNodes[0]);
        }

        void parseCurlyBrackets(ParseTreeNode node)
        {
            parseLineList(node.ChildNodes[0]);
        }

        void parseExpression(ParseTreeNode node)
        {
            switch (node.Term.Name)
            {
                case "identifier":
                    emitIdentifier(node, false);
                    break;

                case "number":
                    emitNumber(node);
                    break;

                case "stringLiteral":
                    emitStringLiteral(node);
                    break;

                case "brackets":
                    parseBrackets(node);
                    break;

                case "mathExpression":
                case "binaryExpression":
                    emitBinaryExpression(node);
                    break;

                case "call":
                    emitCall(node, false);
                    break;

                default:
                    throw new Exception(String.Format("Unknown expression term {0}", node.Term.Name));
            }
        }

        void parseLine(ParseTreeNode node)
        {
            switch (node.Term.Name)
            {
                case "call":
                    emitCall(node, false);
                    break;

                case "threadCall":
                    emitCall(node, true);
                    break;

                case "assignment":
                    emitSetVar(node);
                    break;

                case "wait":
                    emitWait(node);
                    break;

                case "returnStatement":
                    emitReturn(node);
                    break;

                case "jumpStatement":
                    emitBreakOrContinue(node);
                    break;

                case "doStatement":
                    emitDoStatement(node);
                    break;

                case "ifStatement":
                    emitIfStatement(node);
                    break;

                case "whileStatement":
                    emitWhileStatement(node);
                    break;

                case "forStatement":
                    emitForStatement(node);
                    break;

                case "curlyBrackets":
                    parseCurlyBrackets(node);
                    break;

                default:
                    throw new Exception(String.Format("Unknown line term {0}", node.Term.Name));
            }
        }

        void parseLineList(ParseTreeNode node)
        {
            foreach (ParseTreeNode line in node.ChildNodes)
                parseLine(line);
        }

        void parseFunctionNode(ParseTreeNode node)
        {
            bool isAutoExecuted = node.ChildNodes[0].ChildNodes.Count == 1;
            string functionName = node.ChildNodes[1].Token.ValueString;

            ParseTreeNode parameterNode = node.ChildNodes[2];
            int paramCount = parameterNode.ChildNodes.Count;

            script.addNewFunction(functionName, script.getPos(), paramCount, isAutoExecuted);

            if (paramCount > 0)
            {
                if (isAutoExecuted)
                    throw new ScriptException(String.Format("Auto execute function {0} cannot have parameters."), node);
            }
            
            foreach (ParseTreeNode parameter in parameterNode.ChildNodes)
            {
                script.currFunc().addLocalVar(parameter.Token.ValueString);
            }
            
            parseCurlyBrackets(node.ChildNodes[3]);

            script.emitOpcode(OpcodeList.op_end);
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

            try
            {
                foreach (ParseTreeNode node in tree.Root.ChildNodes)
                {
                    parseFunctionNode(node);
                }
            }
            catch (ScriptException)
            {
                // Custom exception does all the work, any coding related exceptions won't be caught though
                return false;
            }
            
        
            script.writeScript(outPath);

            return true;
        }

        private class scriptInfo
        {
            private List<byte> compiledScript;
            public ScriptHeader header;

            private Dictionary<string, functionInfo> functionDict;
            private Dictionary<string, ImportTableValue> importTable;
            private Dictionary<string, StringTableValue> stringTable;
            
            private functionInfo? currentFunction;

            public scriptInfo()
            {
                compiledScript = new List<byte>();
                functionDict = new Dictionary<string, functionInfo>();
                importTable = new Dictionary<string, ImportTableValue>();
                stringTable = new Dictionary<string, StringTableValue>();

                header = new ScriptHeader();
                
                currentFunction = null;

                // Init header, will be overwritten later
                emitInt(0);
                emitInt(0);
                emitInt(0);
                emitInt(0);
                emitInt(0);
                emitInt(0);
            }

            public void writeScript(string outputFilePath)
            {
                int stringTablePtr = getPos();
                foreach (StringTableValue stringValue in stringTable.Values)
                {
                    emitString(stringValue.str);
                    emitInt(stringValue.count);

                    foreach(int ptr in stringValue.instructionPtrList)
                    {
                        emitInt(ptr);
                    }
                }

                int exportTablePtr = getPos();
                foreach (functionInfo exportValue in functionDict.Values)
                {
                    emitString(exportValue.info.name);
                    emitInt(exportValue.info.bytecodePtr);
                    emitInt(exportValue.info.paramCount);
                    emitBool(exportValue.info.isAutoExecuted);
                }

                int importTablePtr = getPos();
                foreach (ImportTableValue importValue in importTable.Values)
                {
                    emitString(importValue.name);
                    emitInt(importValue.count);

                    foreach (int ptr in importValue.instructionPtrList)
                    {
                        emitInt(ptr);
                    }
                }

                replaceBytes(0x00, BitConverter.GetBytes(stringTablePtr));
                replaceBytes(0x04, BitConverter.GetBytes(stringTable.Count));

                replaceBytes(0x08, BitConverter.GetBytes(exportTablePtr));
                replaceBytes(0x0C, BitConverter.GetBytes(functionDict.Count));

                replaceBytes(0x10, BitConverter.GetBytes(importTablePtr));
                replaceBytes(0x14, BitConverter.GetBytes(importTable.Count));

                File.WriteAllBytes(outputFilePath, compiledScript.ToArray());
            }

            public functionInfo? getFunction(string name)
            {
                if (!functionDict.ContainsKey(name))
                    return null;

                return functionDict[name];
            }

            public void addNewFunction(string name, int bytecodeStart, int paramCount, bool isAutoExecuted)
            {
                if (functionDict.ContainsKey(name))
                    throw new ScriptException(String.Format("Cannot have multiple functions named {0}", name));

                functionDict.Add(name, new functionInfo(name, bytecodeStart, paramCount, isAutoExecuted));
                currentFunction = functionDict[name];
            }

            public void addImport(string name, int instructionPtr)
            {
                if(importTable.ContainsKey(name))
                {
                    ImportTableValue import = importTable[name];

                    import.count++;
                    import.instructionPtrList.Add(instructionPtr);
                }
                else
                {
                    importTable.Add(name, new ImportTableValue(name, instructionPtr));
                }
            }

            public void addString(string str, int instructionPtr)
            {
                if (stringTable.ContainsKey(str))
                {
                    StringTableValue import = stringTable[str];

                    import.count++;
                    import.instructionPtrList.Add(instructionPtr);
                }
                else
                {
                    stringTable.Add(str, new StringTableValue(str, instructionPtr));
                }
            }

            public functionInfo currFunc()
            {
                if (currentFunction == null)
                    throw new Exception("currentFunctionName was null when calling currFunc.");

                return currentFunction;
            }

            public int getPos()
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

            public void emitBool(bool val)
            {
                if(val)
                    compiledScript.Add((byte)1);
                else
                    compiledScript.Add((byte)0);
            }

            public void emitInt(int val)
            {
                compiledScript.AddRange(BitConverter.GetBytes(val));
            }

            public void emitUInt(uint val)
            {
                compiledScript.AddRange(BitConverter.GetBytes(val));
            }

            public void emitInt64(long val)
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

            public int emitTempInt()
            {
                int pos = getPos();

                compiledScript.AddRange(BitConverter.GetBytes((int)0));

                return pos;
            }

            public readonly int tempJumpVal = 0;
            public readonly long dummyLongVal = 0;
            public readonly byte booleanTrueVal = 1;
            public readonly byte booleanFalseVal = 0;

            public int emitOpcode(OpcodeList opcode, params object[] values)
            {
                emitByte((byte)opcode);

                int dataPos = getPos();

                switch (opcode)
                {
                    case OpcodeList.op_add:
                    case OpcodeList.op_minus:
                    case OpcodeList.op_multiply:
                    case OpcodeList.op_divide:
                    case OpcodeList.op_modulus:
                    case OpcodeList.op_equal:
                    case OpcodeList.op_notEqual:
                    case OpcodeList.op_lessThan:
                    case OpcodeList.op_greaterThan:
                    case OpcodeList.op_lessThanOrEqual:
                    case OpcodeList.op_greaterThanOrEqual:
                    case OpcodeList.op_wait:
                    case OpcodeList.op_end:
                    case OpcodeList.op_pushNull:
                    case OpcodeList.op_or:
                    case OpcodeList.op_xor:
                    case OpcodeList.op_and:
                    case OpcodeList.op_shiftLeft:
                    case OpcodeList.op_shiftRight:
                        break;

                    case OpcodeList.op_getVar:
                    case OpcodeList.op_setVar:
                    case OpcodeList.op_jump:
                    case OpcodeList.op_jumpOnTrue:
                    case OpcodeList.op_jumpOnFalse:
                    case OpcodeList.op_pushInt:
                        emitInt((int)values[0]);
                        break;

                    case OpcodeList.op_pushFloat:
                        emitFloat((float)values[0]);
                        break;

                    case OpcodeList.op_callBuiltin:
                    case OpcodeList.op_pushString:
                        emitInt64((long)values[0]);
                        break;

                    case OpcodeList.op_pushBool:
                        emitByte((byte)values[0]);
                        break;

                    case OpcodeList.op_callScript:
                    case OpcodeList.op_callScriptThreaded:
                        emitByte((byte)values[0]);
                        emitInt((int)values[1]);
                        break;


                    default:
                        throw new Exception(String.Format("emitOpcode: unknown opcode {0}", opcode));
                }

                return dataPos;
            }
        }

        private class functionInfo
        {
            private List<string> localVarList;

            public ExportTableValue info;

            private readonly int SCR_MAX_VARS = 20;

            public functionInfo(string name, int bytecodeStart, int paramCount, bool isAutoExecuted)
            {
                localVarList = new List<string>();

                info = new ExportTableValue(name, bytecodeStart, paramCount, isAutoExecuted);
            }

            public int addLocalVar(string name)
            {
                if(localVarList.Count == SCR_MAX_VARS)
                    throw new ScriptException(String.Format("Cannot add more than {0} local variables!", SCR_MAX_VARS));

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

        private class loopInfo
        {
            public int loopStart;
            public List<int> breakList;
            public List<int> continueList;

            public loopInfo(int loopStart)
            {
                this.loopStart = loopStart;
                breakList = new List<int>();
                continueList = new List<int>();
            }
        }

        private class logicalOperatorsInfo
        {
            public int operatorCount; // operatorCount is incremented every time a new logical operator is parsed, and decremented once parsing is done

            // Item1 is the AND/OR jump opcode's value, Item2 is the operatorCount when Item1 was pushed
            private Stack<(int, int)> ORList;
            private Stack<(int, int)> ANDList;

            public logicalOperatorsInfo()
            {
                operatorCount = 0;

                ORList = new Stack<(int, int)>();
                ANDList = new Stack<(int, int)>();
            }

            // push OR jump opcode's value
            public void pushOR(int jumpOpcodePos)
            {
                ORList.Push((jumpOpcodePos, operatorCount));
            }

            // push AND jump opcode's value
            public void pushAND(int jumpOpcodePos)
            {
                ANDList.Push((jumpOpcodePos, operatorCount));
            }

            // return list of every OR jump opcode that has a higher operatorCount than the current one
            public List<int> popORs()
            {
                List<int> poppable = new List<int>();

                while(ORList.Count > 0)
                {
                    if (ORList.Peek().Item2 > operatorCount)
                        poppable.Add(ORList.Pop().Item1);
                    else
                        break;
                }

                return poppable;
            }

            // return list of every AND jump opcode that has a higher operatorCount than the current one
            public List<int> popANDs()
            {
                List<int> poppable = new List<int>();

                while (ANDList.Count > 0)
                {
                    if (ANDList.Peek().Item2 > operatorCount)
                        poppable.Add(ANDList.Pop().Item1);
                    else
                        break;
                }

                return poppable;
            }

            // reset OR/AND stack
            public void reset()
            {
                ORList.Clear();
                ANDList.Clear();
            }
        }
    }
}
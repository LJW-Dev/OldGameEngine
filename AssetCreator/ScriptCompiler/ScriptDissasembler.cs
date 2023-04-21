using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static AssetCompiler.ScriptDef;

namespace AssetCompiler
{
    public class ScriptDissasembler
    {
        public void dissasemble(string filePath)
        {
            BinaryReader reader = new BinaryReader(File.Open(filePath, FileMode.Open));

            // will add fixups later
            int stringTablePtr = reader.ReadInt32();
            int stringTableCount = reader.ReadInt32();

            int exportTablePtr = reader.ReadInt32();
            int exportTableCount = reader.ReadInt32();

            int importTablePtr = reader.ReadInt32();
            int importTableCount = reader.ReadInt32();

            while(reader.BaseStream.Position < stringTablePtr)
            {
                long opcodePos = reader.BaseStream.Position;
                OpcodeList opcode = (OpcodeList)reader.ReadByte();
                long opcodeDataPos = reader.BaseStream.Position;

                Console.Write("0x{0:X} {1} ", opcodePos, opcode.ToString());

                

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
                        Console.WriteLine();
                        break;

                    case OpcodeList.op_getVar:
                    case OpcodeList.op_setVar:
                    case OpcodeList.op_pushInt:
                        Console.WriteLine("{0}", reader.ReadInt32());
                        break;

                    case OpcodeList.op_pushFloat:
                        Console.WriteLine("{0}", reader.ReadSingle());
                        break;

                    case OpcodeList.op_callBuiltin:
                    case OpcodeList.op_pushString:
                        Console.WriteLine("{0}", reader.ReadInt64());
                        break;

                    case OpcodeList.op_pushBool:
                        Console.WriteLine("{0}", reader.ReadByte());
                        break;

                    case OpcodeList.op_jump:
                    case OpcodeList.op_jumpOnTrue:
                    case OpcodeList.op_jumpOnFalse:
                        Console.WriteLine("0x{0:X}", opcodeDataPos + reader.ReadInt32());
                        break;

                    case OpcodeList.op_callScript:
                    case OpcodeList.op_callScriptThreaded:
                        byte paramCount = reader.ReadByte();
                        long callPos = opcodeDataPos + reader.ReadInt32();
                        Console.WriteLine("0x{0:X}, param count {1}", callPos, paramCount);
                        break;
                }
                
            }
        }
    }
}

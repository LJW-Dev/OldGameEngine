namespace AssetCompiler
{
    public static class ScriptDef
    {
        public enum OpcodeList
        {
            op_voidOpcode,
            op_pushNull,      
            op_pushBool,
            op_pushInt,
            op_pushFloat,
            op_pushString,
            op_jump,                //jump pos is calculated from pos AFTER opcode
            op_jumpOnTrue,          //jump pos is calculated from pos AFTER opcode
            op_jumpOnFalse,         //jump pos is calculated from pos AFTER opcode
            op_callBuiltin,
            op_callScript,          //jump pos is calculated from pos AFTER opcode
            op_callScriptThreaded,  //jump pos is calculated from pos AFTER opcode
            op_end,
            op_wait,
            op_getVar,
            op_setVar,
            op_or,
            op_xor,
            op_and,
            op_shiftLeft,
            op_shiftRight,
            op_add,                 // Var on top of stack is added to the var next on the stack
            op_minus,               // Var next on the stack is subtracted from the var on top of the stack
            op_multiply,            // Var on top of stack is multiplied with the var next on the stack
            op_divide,              // Var on top of stack is divided by the var next on the stack
            op_modulus,             // Var on top of stack is modulo by the var next on the stack
            op_equal,
            op_notEqual,
            op_lessThan,
            op_greaterThan,
            op_lessThanOrEqual,
            op_greaterThanOrEqual
        };

        public static Dictionary<string, int> BuiltinList = new Dictionary<string, int>{
            { "spawnModel", 4 },
            { "debugPrint", 1 },
            { "moveEntity", 4 }
        };

        public class ScriptHeader
        {
            public int stringTablePtr;
            public int exportTablePtr;
            public int importTablePtr;
        }

        public class StringTableValue
        {
            public string str;
            public int count;
            public List<int> instructionPtrList;

            public StringTableValue(string str, int instructionPtr)
            {
                this.str = str;
                this.count = 1;
                this.instructionPtrList = new List<int> { instructionPtr };
            }
        }

        public class ExportTableValue
        {
            public string name;
            public int bytecodePtr;
            public int paramCount;
            public bool isAutoExecuted;

            public ExportTableValue(string name, int bytecodePtr, int paramCount, bool isAutoExecuted)
            {
                this.name = name;
                this.bytecodePtr = bytecodePtr;
                this.paramCount = paramCount;
                this.isAutoExecuted = isAutoExecuted;
            }
        }

        public class ImportTableValue
        {
            public string name;
            public int count;
            public List<int> instructionPtrList;

            public ImportTableValue(string name, int instructionPtr)
            {
                this.name = name;
                this.count = 1;
                this.instructionPtrList = new List<int> { instructionPtr };
            }
        }
    }
}

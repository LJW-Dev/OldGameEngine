using AssetCompiler;
using System;
using System.IO;

namespace MyApp // Note: actual namespace depends on the project name.
{
    internal class Program
    {
        static void Main(string[] args)
        {
            //if(args.Length == 0)
            //{
            //    Console.WriteLine("Incorrect parameters.");
            //    return;
            //}
            //
            //if (args[0] == "comp")
            //{
                ScriptCompiler compiler = new ScriptCompiler();
                bool result = compiler.CompileScript(@"E:\Coding\Projects\Engines\GameEngine\Engine\x64\Debug\assets\scripts\main.txt", @"E:\Coding\Projects\Engines\GameEngine\Engine\x64\Debug\assets\scripts\main.script");
            //}
            //else if (args[0] == "disasm")
            //{
            if(result)
            {
                ScriptDissasembler dissasembler = new ScriptDissasembler();
                dissasembler.dissasemble(@"E:\Coding\Projects\Engines\GameEngine\Engine\x64\Debug\assets\scripts\main.script");
            }
                
            //}
            
        }

        
    }
}
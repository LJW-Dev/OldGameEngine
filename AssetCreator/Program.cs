using AssetCompiler;
using System;
using System.IO;

namespace MyApp // Note: actual namespace depends on the project name.
{
    internal class Program
    {
        static void Main(string[] args)
        {
            ScriptCompiler compiler = new ScriptCompiler();

            compiler.CompileScript(@"E:\Coding\Projects\Engines\GameEngine\Engine\x64\Debug\assets\scripts\main.txt", @"E:\Coding\Projects\Engines\GameEngine\Engine\x64\Debug\assets\scripts\main.script");
        }

        
    }
}
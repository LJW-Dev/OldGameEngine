



//int popInteger()
//{
//	int val = scrVM.currStack->val.t_int;
//
//	scrVM.currStack--;
//	return val;
//}
//
//float popFloat()
//{
//	float val = scrVM.currStack->val.t_float;
//
//	scrVM.currStack--;
//	return val;
//}
//
//void pushInteger(int val)
//{
//	scrVM.currStack++;
//
//	scrVM.currStack->type = SCRIPT_INTEGER;
//	scrVM.currStack->val.t_int = val;
//}
//
//void pushFloat(float val)
//{
//	scrVM.currStack++;
//
//	scrVM.currStack->type = SCRIPT_FLOAT;
//	scrVM.currStack->val.t_float = val;
//}
//
//void allignScriptPos()
//{
//	scrVM.scriptPos = (scrVM.scriptPos + 3) & 0xFFFFFFFC;
//}
//
//
//
//void executeScript(char* scrName)
//{
//	XScript* script = findAsset(XASSET_SCRIPT, scrName).Script;
//	
//	if (script == NULL)
//	{
//		printf("cant find script %s\n", scrName);
//		return;
//	}
//
//	char* scriptPos = script->script;
//	while (true)
//	{
//		char* currWord = scriptPos;
//		
//		while(*scriptPos != ' ' || *scriptPos != '\n')
//	}
//}
//
//void assembleScript(FILE* script)
//{
//
//}


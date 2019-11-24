#include <fstream>  
#include <string>  
#include <cstring>
#include <iostream>  
#include "strip.h"
#include "Spill_Count.h"
#include "Output_Excel.h"
#include "Branch_Jump_Insn.h"
using namespace std;


int Spill_Count(string filepath,string Function_Output_Excel_path, string Loop_Output_Excel_path)
{
	ifstream in(filepath);
	string File_Name;
	string Line;
	string Spill_Symbol = "(%";
	string Function_Symbol = "0000000";
	string Function_Name_Symbol = "<";
	int Function_Begin;
	int Line_Num = 1;
	int Function_Num = 0;
	int Loop_Num = 0;
	Function Function_List[Max_Fun_Num] = {0};
	Loop Loop_List[Max_Loop_Num];
	string Backwards_Jump[Max_Loop_Num][2]; //The first dimension represents the number of rows where the jump is, and the second dimension represents the number of rows where the jump is
	int Backwards_Jump_Num = 0;
	int Function_List_size=0;
	int Loop_List_size = 0;
	if (in)
	{
		cout<< "File opened successfully" <<endl;
		while (getline(in, Line))
		{
			int Current_Function = Function_Num;
			Line_Num += 1;
			int	begin = 0;
			int strline = Line.size();
			if (strline > 0)
			{
				Line = strip(Line, " ");//Delete any extra space before and after the line
				//Count register spill by Function
				Function_Begin = Line.find(Function_Symbol);
				if (Function_Begin >= 0)
				{
					cout << "We find a Function:" << endl;
					cout << "The functiom number is :"<< Current_Function << endl;
					Function_List[Current_Function].Begin = Line_Num;
					int Function_Name_Begin = Line.find(Function_Name_Symbol);
					string Line_temp = Line.substr(Function_Name_Begin + 1);
					Function_List[Current_Function].Name = rstrip(Line_temp, ">:");
					cout << "Function name is :"<< Function_List[Current_Function].Name << endl;
					Function_Num += 1;
					Function_List_size++;
				}
				int Spill_Begin = Line.find(Spill_Symbol);
				if (Spill_Begin > 0 && Current_Function > 0)
				{
					Function_List[Current_Function - 1].Split_Num += 1;
				}

				if (Current_Function > 0)
				{
					Function_List[Current_Function - 1].Line_Num += 1;
				}
				//Count register spill by loop
				
				for (int i = 0; i < Branch_Jump_Insn_size;i++)
				{
					if(Line.size()>40)
					{
						string Insn = strip(Line.substr(30, 7), " ");//Gets the instruction for the current line
						//printf("%d", Branch_Jump_Insn->size());
						if (Branch_Jump_Insn[i] == Insn)
						{
							try
							{
								string Self_Line_Num_str = Line.substr(0, 6);
								string Jump_Line_Num_str = Line.substr(37, 6);
								int Self_Line_Num_int = stoi(Self_Line_Num_str, nullptr, 16);
								int Jump_Line_Num_int = stoi(Jump_Line_Num_str, nullptr, 16);
								/*
								There is an instruction to jump to the previous code, but this does not prove that this is a return, 
								because it does not prove that there is no other path to jump to this area
								*/
								if (Jump_Line_Num_int < Self_Line_Num_int)
								{
									cout << "We find a Loop:" << endl;				
									int Current_Loop = Loop_Num;
									cout << "The loop number is :" << Current_Loop << endl;
									Loop_List[Current_Loop].Fun_Name = Function_List[Current_Function-1].Name;
									Loop_List[Current_Loop].Begin =  Jump_Line_Num_str;
									Loop_List[Current_Loop].End = Self_Line_Num_str;
									cout << "Start counting the overflow information for this loop" << endl;
									Loop_List[Current_Loop].Split_Num = Spill_Count_By_Line_Num(filepath, Jump_Line_Num_str,Self_Line_Num_str);
									Loop_List[Current_Loop].Loop_No = Loop_Num;
									cout << "The spill information of this loop has been counted!" << endl;
									//(Function_List[Current_Function].loop)->Assembly_Begin = &Self_Line_Num_str;
									//Function_List[Current_Function].loop->Assembly_End = &Jump_Line_Num_str;
									Loop_Num += 1;
									Loop_List_size++;
									//printf("2");
								}
								else
								{
									//The jump is backwards
									Backwards_Jump[Backwards_Jump_Num][0] = Self_Line_Num_str;
									Backwards_Jump[Backwards_Jump_Num][1] = Jump_Line_Num_str;
									Backwards_Jump_Num++;
								}


							}
							catch (overflow_error e)
							{
							}
						}

					}
				}
				//cout << Line << endl;
			}
		}
		/*
		Remove nodes that are not return edges from the existing Loop List
		*/
		for (int i = 0; i < Backwards_Jump_Num; ++i)
		{
			if (Loop_List[i].Begin > Backwards_Jump[i][0] && Loop_List[i].Begin < Backwards_Jump[i][1])
			{
				cout<<"Area " <<Loop_List[i].Loop_No <<" has other entries. It's not a loop" << endl;
				//This indicates that there are other entrances to the area, which is not a loop
				Loop_List[i].Loop_No = -1;
				
			}
		}
		cout << "Register spill information has been calculated in units of functions" << endl;
		cout << "Register spill information has been calculated in units of loops" << endl;
		Function_Output_Excel(Function_Output_Excel_path, Function_List, Function_List_size);
		

		Loop_Output_Excel(Loop_Output_Excel_path, Loop_List, Loop_List_size);
	}
	else
	{
		cout << "no such file" << endl;
	}
	return 0;
}


int Spill_Count_By_Line_Num(string filepath,string Begin_Line_Num_str, string End_Line_Num_str)
{
	
	ifstream in2(filepath);
	string File_Name;
	string Line;
	string Spill_Symbol = "(%";
	int Line_Num = 1;
	int Loop_Num = 0;
	int Split_Num = 0;
	bool Count_Begin = false;
	if (in2)
	{
		while (getline(in2, Line))
		{
			int strline = Line.size();
			if (strline > 0) 
			{
				Line = strip(Line, " ");
				string Line_Num_str = Line.substr(0, 6);
				if (Line_Num_str == Begin_Line_Num_str)
				{
					Count_Begin = true;
				}
				if (Count_Begin)
				{
					int Spill_Begin = Line.find(Spill_Symbol);
					if (Spill_Begin > 0)
					{
						Split_Num += 1;
					}
				}
				if (Line_Num_str == End_Line_Num_str)
				{
					break;
				}
			}
		}
	}
	else
	{
		cout << "no such file" << endl;
	}
	return Split_Num;

}

/*
void Delete_Non_Return_Edges(Loop Loop_List[], int Loop_List_size)
{
	string Line;
	string Spill_Symbol = "(%";
	int Line_Num = 1;
	int Loop_Num = 0;
	int Split_Num = 0;
	bool Count_Begin = false;
}
*/
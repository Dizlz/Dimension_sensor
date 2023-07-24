#include <iostream>
#include "../include/wiringPi.h"
#include <sys/time.h>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <pthread.h>

using namespace std;

class CMyCriticalSection
{
public:
	CMyCriticalSection()
	{
		::pthread_mutex_init(&m_mutex, NULL); 
	}

	~CMyCriticalSection()
	{
		::pthread_mutex_destroy(&m_mutex);
	}
	void Enter() const
	{
		::pthread_mutex_lock(&m_mutex);
	}

	void Leave()
	{
		::pthread_mutex_unlock(&m_mutex);
	}
private:
	mutable pthread_mutex_t m_mutex;
}cmyCriticalSection;

struct GabaritInterrupt
{
	timeval t;
	bool side;
	GabaritInterrupt(timeval time, bool Side)
	{
		t = time;
		side = Side;
	}
};

class Settings
{	
public:
	Settings()
	{
		ifstream SettingsFile;
		SettingsFile.open("Settings.txt", ios_base::in);
		string value, key;
		if (SettingsFile.is_open())
		{
			while (!SettingsFile.eof())
			{
				SettingsFile >> key;
				SettingsFile >> value;
				cout << key << " " << value << endl;
				config[key] = value;
			}
			SettingsFile.close();
		}
		else
		{
			string comand;
			cout << "Settings not open!" << endl << "Write \"Exit\"" << endl;
			cin >> comand;
			while (comand != "Exit")
			{
				cout << "Invalid word" << endl;
				cin >> comand;
			}
			exit(0);
		}
	}
	string GetStoragePath();
	int GetPin(string key); 	
private:
	map <string, string> config;	
}settings;

string Settings::GetStoragePath()
{
	return config["StoragePath"];
}

int Settings::GetPin(string key)
{
	return atoi(config[key].c_str());
}

class Data
{
protected:
	vector <GabaritInterrupt> GabaritInterruptVector;	
	timeval trainId;
public:
	void write_interrupt_time(timeval time, bool side);
	void write_to_file(string base_dir);	
	void TrainArriveInterrupt(timeval time);
}data;

void Data::write_to_file(string base_dir)//Запись данных в файл
{			
	string path = base_dir + "/" + to_string(trainId.tv_sec) + ".csv";
	ofstream viol_file;
	viol_file.open(path, ios_base::out);
	cout << "Size = " << GabaritInterruptVector.size() << endl;
	int i;
	if (viol_file.is_open()) 
	{		
		for (i = 0; i < GabaritInterruptVector.size(); i++)
		{				
			viol_file << (GabaritInterruptVector[i].t.tv_sec - trainId.tv_sec) * 1000000ULL + GabaritInterruptVector[i].t.tv_usec - trainId.tv_usec << "\t" << (GabaritInterruptVector[i].side == true ? "Left" : "Right") << endl;
		}
		viol_file.flush();
		viol_file.close();
	}
	data.trainId.tv_sec = 0;
	data.trainId.tv_usec = 0;
	GabaritInterruptVector.clear();
}

void Data::write_interrupt_time(timeval time, bool side)//Запись данных в вектор
{	
	cmyCriticalSection.Enter();
	data.GabaritInterruptVector.push_back(GabaritInterrupt(time, side));		
	cmyCriticalSection.Leave();
}

void Data::TrainArriveInterrupt(timeval time)//Прибытие поезда
{			
	trainId = time;
	cout << "Train arrive Interrupt! " << endl;
	GabaritInterruptVector.clear();
}

void TrainLeaveInterrupt()//Отъезд поезда
{	
	cout << "Train leave Interrupt! " << endl;
	data.write_to_file(settings.GetStoragePath());	
}

void TrainInterrupt()//Прерывание поезда
{
	timeval time;
	gettimeofday(&time, NULL);
	int p1 = digitalRead(0);
	if (p1 == 1)
	{		
		data.TrainArriveInterrupt(time);
	}
	else
	{
		TrainLeaveInterrupt();
	}
}

void leftdimensionInterrupt()
{
	timeval time;
	gettimeofday(&time, NULL);
	if (digitalRead(0) == 1)
	{
		cout << "Left " << endl;
		data.write_interrupt_time(time, true);
	}	
}

void RightdimensionInterrupt()
{	
	timeval time;
	gettimeofday(&time, NULL);
	if (digitalRead(0) == 1)
	{
		cout << "Right " << endl;
		data.write_interrupt_time(time, false);
	}
}

void init()//Инициализация WiringPi
{
	int train = settings.GetPin("TrainPin"), left = settings.GetPin("LeftPin"), right = settings.GetPin("RightPin");
	
	wiringPiSetup();
	//Датчик поезда
	pinMode(train, INPUT);
	pullUpDnControl(0, PUD_DOWN);
	wiringPiISR(0, INT_EDGE_BOTH, TrainInterrupt);
	//Датчики габаритов
	pinMode(left, INPUT);
	pinMode(right, INPUT);
	pullUpDnControl(1, PUD_UP);
	pullUpDnControl(2, PUD_UP);
	wiringPiISR(1, INT_EDGE_FALLING, leftdimensionInterrupt);
	wiringPiISR(2, INT_EDGE_FALLING, RightdimensionInterrupt);
}

int main()
{		
	init();	
	cout << "Start programm" << endl << "Write \"Stop\" to stop programm" << endl;
	string comand;	
	cin >> comand;
	while (comand != "Stop")
	{
		cout << "Invalid word" << endl;
		cin >> comand;
	}	
	return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "task_agency.h"
#include "task_def.h"
#include "api.h"

using namespace std;

class TaskSchedule
{
  public:
  TaskSchedule(sm_msg* msg, TaskAgency* taskAg, BlockingQueue<PictureInfor>* PictureList);
  ~TaskSchedule();

  void ModeSwitch();
  void Reinforcement();
  
  private:
  sm_msg* msg;
  rl_buffer rbuf;
  BlockingQueue<PictureInfor>* m_PictureList; 
  TaskAgency* taskAg;
};

#pragma once
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <memory>
#include <fcntl.h>
#include <cstring>//memset

//-----STL-----
#include <vector>
#include <string>
#include <functional>

void sys_err(const char *str);
void process(int a);

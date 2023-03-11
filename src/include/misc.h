#pragma once

#include <string>
#include <vector>

std::string execCmdSync(const std::vector<std::string>& command);
void bound2little();
std::string getTopApp();
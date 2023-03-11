#pragma once

#include <string>
#include <vector>

std::string execCmdSync(const std::string&, const std::vector<std::string>&);
void bound2little();
std::string getTopApp();
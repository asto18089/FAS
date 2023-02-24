#pragma once

void start_close_others();

// Edit and Lock a file
template <typename T>
bool Lockvalue(const char *location, T value)
{
    chmod(location, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IROTH);
    ofstream fd(location, ios::out | ios::trunc);
    if (!fd)
        return false;
    fd << value;
    chmod(location, S_IRUSR | S_IRGRP | S_IROTH);
    return true;
}

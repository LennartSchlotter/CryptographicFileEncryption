#ifndef HANDLER_H

#define HANDLER_H

#include <span>
struct Request;
struct Result;

void parse(std::span<const char* const> args);
void dispatch(const Request& request);

#endif
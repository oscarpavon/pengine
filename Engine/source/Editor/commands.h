#ifndef EDITOR_COMMAND_H
#define EDITOR_COMMAND_H

void parse_command(const char* command);
void text_input_mode();
unsigned char command_text_buffer[100];

#endif
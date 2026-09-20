#ifndef SMTP_H
#define SMTP_H

int smtp_send_report(char *server_ip, int server_port, const char *sender, const char *recipient, const char *subject, const char *body);

#endif
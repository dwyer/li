#include "li.h"

#include <netdb.h> /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h> /* read, write, close */

typedef struct {
    LI_OBJ_HEAD;
    int fd;
    struct sockaddr_in addr;
} li_socket_t;

static void socket_close(li_object *obj)
{
    close(((li_socket_t *)obj)->fd);
}

const li_type_t li_type_socket = {
    .name = "socket",
    .deinit = socket_close,
};

static li_object *p_make_client_socket(li_object *args)
{
    li_socket_t *obj;
    li_str_t *node, *service;
    struct hostent *hostent;
    int ai_family = AF_INET,
        ai_socktype = SOCK_STREAM,
        ai_flags = AI_V4MAPPED | AI_ADDRCONFIG, /* TODO: use these flags */
        ai_protocol = IPPROTO_IP;
    li_parse_args(args, "ss?iiii", &node, &service,
            &ai_family, &ai_socktype, &ai_flags, &ai_protocol);
    hostent = gethostbyname(li_string_bytes(node));
    if (hostent == NULL)
        li_error_f("ERROR, no such host");
    obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_socket);
    obj->fd = socket(ai_family, ai_socktype, ai_protocol);
    if (obj->fd < 0)
        li_error("ERROR opening socket", NULL);
    /* init address */
    obj->addr.sin_family = ai_family;
    obj->addr.sin_port = htons(atoi(li_string_bytes(service)));
    memcpy(&obj->addr.sin_addr.s_addr, hostent->h_addr, hostent->h_length);
    memset(&obj->addr.sin_zero, 0, sizeof(obj->addr.sin_zero));
    /* connect the socket */
    if (connect(obj->fd, (struct sockaddr *)&obj->addr, sizeof(obj->addr)) < 0)
        li_error_f("ERROR connecting");
    /* create the object */
    return (li_object *)obj;
}

static li_object *p_is_socket(li_object *args)
{
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_type(obj) == &li_type_socket);
}

static li_object *p_socket_accept(li_object *args)
{
    li_object *obj;
    li_socket_t *sock;
    int fd;
    socklen_t len;
    li_parse_args(args, "o", &obj);
    sock = (li_socket_t *)obj;
    len = sizeof(sock->addr);
    fd = accept(sock->fd, (struct sockaddr *)&sock->addr, &len);
    close(fd);
    return obj;
}

static li_object *p_socket_send(li_object *args)
{
    li_object *obj;
    li_str_t *str;
    char *message;
    int sent, total;
    li_parse_args(args, "os", &obj, &str);
    message = li_string_bytes(str);
    total = strlen(message);
    sent = 0;
    do {
        int n = write(((li_socket_t *)obj)->fd, message + sent, total - sent);
        if (n < 0)
            li_error_f("ERROR writing message to socket");
        if (n == 0)
            break;
        sent += n;
    } while (sent < total);
    return (li_object *)li_num_with_int(sent);
}

static li_object *p_socket_recv(li_object *args)
{
    li_object *obj;
    int size;
    int n;
    char buf[BUFSIZ];
    li_parse_args(args, "oi", &obj, &size);
    n = read(((li_socket_t *)obj)->fd, buf, size-1);
    if (n < 0)
        li_error_f("ERROR reading from socket");
    buf[n] = '\0';
    return (li_object *)li_string_make(buf);
}

static li_object *p_socket_shutdown(li_object *args)
{
    li_object *obj;
    int how;
    li_parse_args(args, "oi", &obj, &how);
    if (shutdown(((li_socket_t *)obj)->fd, how))
        li_error("shutdown error", args);
    return NULL;
}

static li_object *p_socket_close(li_object *args)
{
    li_object *obj;
    li_parse_args(args, "o", &obj);
    socket_close(obj);
    return obj;
}

#define li_def_var(env, name, obj) \
    li_env_append(env, li_symbol(name), obj)

#define li_def_fun(env, name, proc) \
    li_def_var(env, name, li_primitive_procedure(proc))

#define li_def_int(env, name, i) \
    li_def_var(env, name, (li_object *)li_num_with_int(i))

extern void li_define_socket_functions(li_env_t *env)
{
    li_def_fun(env, "make-client-socket", p_make_client_socket);
    li_def_fun(env, "socket?", p_is_socket);
    li_def_fun(env, "socket-accept", p_socket_accept);
    li_def_fun(env, "socket-send", p_socket_send);
    li_def_fun(env, "socket-recv", p_socket_recv);
    li_def_fun(env, "socket-shutdown", p_socket_shutdown);
    li_def_fun(env, "socket-close", p_socket_close);
    li_def_int(env, "*af-unspec*", AF_UNSPEC);
    li_def_int(env, "*af-inet*", AF_INET);
    li_def_int(env, "*af-inet6*", AF_INET6);
    li_def_int(env, "*sock-stream*", SOCK_STREAM);
    li_def_int(env, "*sock-dgram*", SOCK_DGRAM);
    li_def_int(env, "*ai-canonname*", AI_CANONNAME);
    li_def_int(env, "*ai-numerichost*", AI_NUMERICHOST);
    li_def_int(env, "*ai-v4mapped*", AI_V4MAPPED);
    li_def_int(env, "*ai-all*", AI_ALL);
    li_def_int(env, "*ai-addrconfig*", AI_ADDRCONFIG);
    li_def_int(env, "*ipproto-ip*", IPPROTO_IP);
    li_def_int(env, "*ipproto-tcp*", IPPROTO_TCP);
    li_def_int(env, "*ipproto-udp*", IPPROTO_UDP);
    li_def_int(env, "*msg-peek*", MSG_PEEK);
    li_def_int(env, "*msg-oob*", MSG_OOB);
    li_def_int(env, "*msg-waitall*", MSG_WAITALL);
    li_def_int(env, "*shut-rd*", SHUT_RD);
    li_def_int(env, "*shut-wr*", SHUT_WR);
    li_def_int(env, "*shut-rdwr*", SHUT_RDWR);
}

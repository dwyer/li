#include "li.h"
#include "li_lib.h"

#include <netdb.h> /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h> /* read, write, close */

/* Based on https://srfi.schemers.org/srfi-106/srfi-106.html */

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
        li_error_f("bad host: ~a", node);
    obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_socket);
    obj->fd = socket(ai_family, ai_socktype, ai_protocol);
    if (obj->fd < 0)
        li_error_f("ERROR opening socket");
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
    close(fd); /* TODO finish writing this */
    return obj;
}

static li_object *p_socket_send(li_object *args)
{
    li_object *obj;
    li_str_t *str;
    int flags = 0;
    char *message;
    int sent, total;
    li_parse_args(args, "os?i", &obj, &str, &flags);
    message = li_string_bytes(str);
    total = strlen(message);
    sent = 0;
    do {
        int n = send(((li_socket_t *)obj)->fd, message + sent, total - sent, flags);
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
    li_str_t *res;
    li_object *obj;
    int size, flags = 0;
    int n;
    char _buf[BUFSIZ];
    char *buf = _buf;
    li_parse_args(args, "oi?i", &obj, &size, &flags);
    if (size > BUFSIZ)
        buf = li_allocate(NULL, size, sizeof(*buf));
    n = recv(((li_socket_t *)obj)->fd, buf, size, flags);
    if (n < 0)
        li_error_f("couldn't read from socket");
    buf[n] = '\0';
    res = li_string_make(buf);
    if (buf != _buf)
        free(buf);
    return (li_object *)res;
}

static li_object *p_socket_shutdown(li_object *args)
{
    li_object *obj;
    int how;
    li_parse_args(args, "oi", &obj, &how);
    if (shutdown(((li_socket_t *)obj)->fd, how))
        /* li_error("shutdown error", args); */
        ;
    return NULL;
}

static li_object *p_socket_close(li_object *args)
{
    li_object *obj;
    li_parse_args(args, "o", &obj);
    socket_close(obj);
    return obj;
}

extern void lilib_load(li_env_t *env)
{
    lilib_defproc(env, "make-client-socket", p_make_client_socket);
    lilib_defproc(env, "socket?", p_is_socket);
    lilib_defproc(env, "socket-accept", p_socket_accept);
    lilib_defproc(env, "socket-send", p_socket_send);
    lilib_defproc(env, "socket-recv", p_socket_recv);
    lilib_defproc(env, "socket-shutdown", p_socket_shutdown);
    lilib_defproc(env, "socket-close", p_socket_close);
    lilib_defint(env, "*af-unspec*", AF_UNSPEC);
    lilib_defint(env, "*af-inet*", AF_INET);
    lilib_defint(env, "*af-inet6*", AF_INET6);
    lilib_defint(env, "*sock-stream*", SOCK_STREAM);
    lilib_defint(env, "*sock-dgram*", SOCK_DGRAM);
    lilib_defint(env, "*ai-canonname*", AI_CANONNAME);
    lilib_defint(env, "*ai-numerichost*", AI_NUMERICHOST);
    lilib_defint(env, "*ai-v4mapped*", AI_V4MAPPED);
    lilib_defint(env, "*ai-all*", AI_ALL);
    lilib_defint(env, "*ai-addrconfig*", AI_ADDRCONFIG);
    lilib_defint(env, "*ipproto-ip*", IPPROTO_IP);
    lilib_defint(env, "*ipproto-tcp*", IPPROTO_TCP);
    lilib_defint(env, "*ipproto-udp*", IPPROTO_UDP);
    lilib_defint(env, "*msg-peek*", MSG_PEEK);
    lilib_defint(env, "*msg-oob*", MSG_OOB);
    lilib_defint(env, "*msg-waitall*", MSG_WAITALL);
    lilib_defint(env, "*shut-rd*", SHUT_RD);
    lilib_defint(env, "*shut-wr*", SHUT_WR);
    lilib_defint(env, "*shut-rdwr*", SHUT_RDWR);
}

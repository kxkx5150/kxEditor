#include "LinkedList.h"


long long LinkedList ::forward_traverse(BOOL printnd)
{
    node* trav;
    trav = front;
    long long count = 0;
    OutputDebugString(L"\n--------------check forward---\n");

    while (trav != NULL) {
        if (trav->data) {
            count++;
            if (printnd)
                print_node_data(trav);
        }

        if (end == trav) {
            OutputDebugString(L"\n--------------end\n");
        }
        trav = trav->next;
    }

    _RPTN(_CRT_WARN, "\ncount   : %ld\n\n", count);
    OutputDebugString(L"\n--------------forward ok---\n");

    return count;
}
long long LinkedList ::backward_traverse(BOOL printnd)
{
    node* trav;
    trav = end;
    long long count = 0;
    OutputDebugString(L"\n--------------check backward---\n");

    while (trav != NULL) {
        if (trav->data) {
            count++;
            if (printnd)
                print_node_data(trav);
        }

        if (front == trav) {
            OutputDebugString(L"\n--------------front\n");
        }
        trav = trav->prev;
    }

    _RPTN(_CRT_WARN, "\ncount   : %ld\n\n", count);
    OutputDebugString(L"\n--------------backward ok---\n");

    return count;
}
void LinkedList::check_link(BOOL printnd)
{
    OutputDebugString(L"\n--------------check link---\n\n");

    long long fcnt = forward_traverse(printnd);
    long long bcnt = backward_traverse(printnd);
    if (fcnt != bcnt)
        OutputDebugString(L"\n************************\n************ Error!\n************************\n\n");
    else {
        OutputDebugString(L"\nlink ok\n\n");
    }

    _RPTN(_CRT_WARN, "forward  count :%ld\n", fcnt);
    _RPTN(_CRT_WARN, "backward count :%ld\n", bcnt);
    OutputDebugString(L"\n--------------check link---\n\n");
}
void LinkedList::print_node_data(node* trav)
{
    if (!trav || !trav->data)
        return;

    BYTE* tmp = trav->data;
    long long len = trav->datalen;
    TCHAR* buff = new TCHAR[len + 1];
    int buflen;
    gettext(tmp, len, buff, &buflen);
    buff[buflen] = '\0';

    OutputDebugString(L"\n--------------\n");
    OutputDebugString(L"\nUTF-16         : ");
    OutputDebugString(buff);
    _RPTN(_CRT_WARN, "\nUTF-8          : %s\n", tmp);
    _RPTN(_CRT_WARN, "utf8 len       : %ld\n", trav->datalen);

    int crlen = CRLF_size(buff, buflen);
    _RPTN(_CRT_WARN, "utf-16 crlf    : %d\n", crlen);

    crlen = CRLF_size(tmp, trav->datalen);
    _RPTN(_CRT_WARN, "utf-8  crlf    : %d\n", crlen);

    delete[] buff;
}

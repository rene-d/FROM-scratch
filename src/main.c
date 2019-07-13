#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <glib.h>

// GLib examples
// https://github.com/steshaw/gtk-examples/tree/master/ch02.glib
int main_hash();
int main_list();
int main_sortlist();
int main_tree();

// jansson example
// https://jansson.readthedocs.io/en/2.12/tutorial.html#the-program
int main_jansson(int argc, char *argv[]);


int main()
{
    extern const char welcome[];
    fputs(welcome, stdout);

    printf("\033[1;32m");
    printf("\ngethostbyname ===>\n");
    struct hostent *e = gethostbyname("dns.google"); // 8.8.8.8 et 8.8.4.4
    printf("hostent: %p\n", e);
    if (e != NULL)
    {
        printf("name: %s", e->h_name);
        unsigned int i = 0;
        while (e->h_addr_list[i] != NULL)
        {
            printf(" %s", inet_ntoa(*(struct in_addr *)(e->h_addr_list[i])));
            i++;
        }
        printf("\n");
    }

    printf("\nGList ===>\n");
    GList *list = NULL;

    for (int i = 0; i < 10; ++i)
    {
        char *s = (char *) malloc(20);
        sprintf(s, "element %d", i + 1);
        list = g_list_append(list, s);
    }

    GList *it = g_list_first(list);
    while (it != NULL)
    {
        printf("%s\n", (char *)it->data);
        free(it->data);
        it = g_list_next(it);
    }
    g_list_free(list);

    printf("\033[0m");

    // ---------------------------------------------------------------------

    printf("\033[1;34m");

    printf("\nhash ===>\n");
    main_hash();

    printf("\nlist ===>\n");
    main_list();

    printf("\nsortlist ===>\n");
    main_sortlist();

    printf("\ntree ===>\n");
    main_tree();

    printf("\033[0m");

    // ---------------------------------------------------------------------

    printf("\033[1;33m");
    printf("\njansson ===>\n");
    char *args[] = {NULL, "rene-d", "pypim"};
    main_jansson(3, args);
    printf("\033[0m");
}

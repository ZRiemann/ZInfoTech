static int g_tstso_int;

int tstso_add(int a, int b){
    g_tstso_int = a+b;
    return g_tstso_int;
}
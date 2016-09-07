#include "hmo.h"
#include "shell.h"

static HmoModule *root;

void hmoRegister(HmoModule *m) {
    m->link(root);
    root = m;
}

void setup() {
    setupShell();
    HmoModule *m;
    for(m=root;m;m=m->next())
        m->setup();
}

void loop() {
    loopShell();
    HmoModule *m;
    for(m=root;m;m=m->next())
        m->loop();
}


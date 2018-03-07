bit boton;
bit presencia;
byte state;

ltl spec {
    [](((state==0) && boton) -> <> (state==1))) && 
    [](((state==1) && presencia) -> <> (state==1))) && 
    [](((state==1) && boton) -> <> (state==0)))

}

active proctype fsm() {
    state = 0;
    do
    :: (state==0) -> atomic {
        if
        :: boton -> state=1;
        :: boton = 0;
        fi
        }
    :: (state==1) -> atomic {
        if
        :: boton -> state=0;
        :: presencia -> state=1;
        :: boton = 0;
        :: presencia = 0;
        fi
    }
    od
}

active proctype entorno() {
    do
    :: boton=1;
    :: presencia=1; 
    :: boton=0; 
    :: presencia=0;
    od
}
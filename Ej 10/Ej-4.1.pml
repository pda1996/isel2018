bit boton1;
bit boton2;
byte state;

ltl spec {
    ([](((state==0) && (boton1) -> (<> (state==1))))) && 
    ([](((state==1) && (boton1) -> (<> (state==0))))) &&
    ([](((state==0) && (boton2) -> (<> (state==1))))) && 
    ([](((state==1) && (boton2) -> (<> (state==0)))))       
}

active proctype fsm() {
    state = 0;
    do
    :: (state==0) -> atomic {
        if
        :: boton1 -> state=1;
        :: boton2 -> state=1;
        :: boton1 = 0;
        :: boton2 = 0;
        fi
        }
    :: (state==1) -> atomic {
        if
        :: boton1 -> state=0;
        :: boton2 -> state=0;
        :: boton1 = 0;
        :: boton2 = 0;
        fi
    }
    od
}

active proctype entorno() {
    do
    :: boton1=1;
    :: boton2=1; 
    :: boton1=0; 
    :: boton2=0;
    od
}
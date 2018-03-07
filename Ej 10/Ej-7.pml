#define timeout true

bit boton;
bit presencia;
byte state;

ltl spec  {
	[] (((state == 0) && boton) -> <> (state == 1)) &&

	[] (((state == 1) && boton) -> <> (state == 2)) &&
	[] (((state == 1) && timeout) -> <> (state == 1)) &&

	[] (((state == 2) && boton) -> <> (state == 3)) &&
	[] (((state == 2) && timeout) -> <> (state == 1)) &&

	[] (((state == 3) && boton) -> <> (state == 4)) &&
	[] (((state == 3) && timeout) -> <> (state == 1)) &&

	[] (((state == 4) && boton) -> <> (state == 5)) &&
	[] (((state == 4) && presencia) -> <> (state == 4)) &&
	
	[] (((state == 5) && boton) -> <> (state == 6)) &&
	[] (((state == 5) && timeout) -> <> (state == 4)) &&

	[] (((state == 6) && boton) -> <> (state == 7)) &&
	[] (((state == 6) && timeout) -> <> (state == 4)) &&

	[] (((state == 7) && boton) -> <> (state == 0)) &&
	[] (((state == 7) && timeout) -> <> (state == 4))
}


active proctype alarma_codigo () {

	state = 0;
	do
	:: (state == 0) -> atomic {
		if
		:: boton -> state = 1; 
        :: boton = 0;
		fi
	}
	:: (state == 1) -> atomic {
		if
		:: boton -> state = 2; 
        :: boton = 0;
		:: timeout -> state = 0;
		fi
	}
	:: (state == 2) -> atomic {
		if
		:: boton -> state = 3; 
        :: boton = 0;
		:: timeout -> state = 0;
		fi
	}
	:: (state == 3) -> atomic {
		if
		:: boton -> state = 4; 
        :: boton = 0;
		:: timeout -> state = 0;
		fi
	}
	:: (state == 4) -> atomic {
		if
		:: boton -> state = 5; 
        :: boton = 0;
		:: presencia -> state = 4;
        :: presencia = 0; 
		fi
	}
	:: (state == 5) -> atomic {
		if
		:: boton -> state = 6; 
        :: boton = 0;
		:: timeout -> state = 4;
		fi
	}
	:: (state == 6) -> atomic {
		if
		:: boton -> state = 7; 
        :: boton = 0;
		:: timeout -> state = 4;
		fi
	}
	:: (state == 7) -> atomic {
		if
		:: boton -> state = 0; 
        :: boton = 0;
		:: timeout -> state = 4;
		fi
	}
	od
}

active proctype entorno () {
	do
	:: boton = 1;
    :: presencia = 1;
	:: boton = 0;
	:: presencia = 0;
	od
}
#define timeout true;
untype = {on, off};
bit cod_correcto, mirar_flags, timeout, codigo_in, cod_incorrecto, presencia;
byte state;

ltl espec{ ([](((state==off) && cod_correcto) -> (<>(state==on))))&&
		   ([](((state==off) && mirar_flags) -> (<>(state==off))))&&
		   ([](((state==off) && timeout) -> (<>(state==off))))&&
		   ([](((state==off) && codigo_in) -> (<>(state==off))))&&
		   ([](((state==off) && cod_incorrecto) -> (<>(state==off))))&&
		   ([](((state==on) && cod_correcto) -> (<>(state==off))))&&
		   ([](((state==on) && mirar_flag) -> (<>(state==on))))&&
		   ([](((state==on) && timeout) -> (<>(state==on))))&&
		   ([](((state==on) && presencia) -> (<>(state==on))))&&
		   ([](((state==on) && cod_incorrecto) -> (<>(state==on))))
		   };


active proctype alarma_8(){
	state = off;

	do

	:: (state==off) -> atomic{
	if
	:: cod_correcto -> state=on; cod_correcto = 0;
	:: mirar_flags -> state = off; mirar_flags =0;
	:: timeout -> state = off; timeout = 0;
	:: codigo_in -> state = off; codigo_in =0;
	fi
	}

	:: (state==on) ->  atomic{
	if
	:: cod_correcto -> state= off; cod_correcto =0;
	:: mirar_flags -> state= off; mirar_flags =0;
	:: timeout -> state= off; timeout =0;
	:: presencia -> state= off; presencia =0;
	:: cod_incorrecto -> state= off; cod_incorrecto =0; 
	fi
	}
	od
}

active proctype entorno(){
	cod_correcto = 1;
	cod_incorrecto = 1;
	presencia = 1;
	timeout = 1;
	mirar_flags = 1;
	codigo_in = 1;
	
	cod_correcto = 0;
	cod_incorrecto = 0;
	presencia = 0;
	timeout = 0;
	mirar_flags = 0;
	codigo_in = 0;
		
}
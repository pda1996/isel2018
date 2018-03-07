#define timeout true;
untype = {on, off, led_on, led_off, code};
bit cod_correcto, mirar_flags, timeout, codigo_in, cod_incorrecto, presencia, code_ready;
byte state;

ltl espec{ ([](((state==code) && cod_correcto) -> (<>(state==code))))&&
		   ([](((state==code) && mirar_flags) -> (<>(state==code))))&&
		   ([](((state==code) && timeout) -> (<>(state==code))))&&
		   ([](((state==code) && codigo_in) -> (<>(state==code))))
};

ltl espec{
		   ([](((state==off) && code_ready) -> (<>(state==on))))&&
		   ([](((state==on) && presencia) -> (<>(state==on))))&&
		   ([](((state==on) && code_ready) -> (<>(state==off))))
};

ltl espec{([](((state== led_off) && presencia)->(<>(state==led_on)))) &&
		 ([](((state== led_on ) && timeout)->(<>(state==led_off)))) &&
		 ([](((state== led_on ) && presencia)->(<>(state==led_on))))}


active proctype alarma_8(){
	state = off;

	do

	:: (state==off) -> atomic{
	if
	:: code_ready -> state=on; code_ready = 0;
	fi
	}

	:: (state==on) ->  atomic{
	if
	:: code_ready-> state= off; code_ready =0;
	:: presencia -> state= off; presencia =0;
	fi
	}

	:: (state==led_off) -> atomic{
	if
	:: presencia -> state= led_on; presencia = 0;
	fi
	}

	:: (state==led_on) ->  atomic{
	if
	:: timeout-> state= off; timeout =0;
	:: presencia -> state= off; presencia =0;
	fi
	}

	:: (state==code) -> atomic{
	if
	:: cod_correcto -> state=code; cod_correcto = 0;
	:: mirar_flags -> state = code; mirar_flags =0;
	:: timeout -> state = code; timeout = 0;
	:: codigo_in -> state = code; codigo_in =0;
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
	code_ready = 1;
	
	cod_correcto = 0;
	cod_incorrecto = 0;
	presencia = 0;
	timeout = 0;
	mirar_flags = 0;
	codigo_in = 0;
	code_ready = 0;	
}

ltl espec{([](((state== OFF)&&presencia)->(<>(state==on)))) &&
		 ([](((state== on )&&timeout)->(<>(state==off))))}

untype = {off, on};
#define timeout true;

bit presencia;
byte state;

active proctype lampara_5(){
	state=off;
	do
	::(state==off) -> atomic{
		if
		:: presencia-> state=on;
		:: presencia = 0;
		fi
	}

	::(state ==on) -> atomic{
		if
		:: presencia-> state=on;
		:: presencia=0;
		fi
	}

	if
	:: timeout-> state=off;
	fi

	}
	od
}


active proctype entorno(){
	do
	:: presencia = 1;
	:: presencia = 0;
	od
}
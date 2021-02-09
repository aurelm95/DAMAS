import os
from selenium import webdriver
from selenium.webdriver.common.keys import Keys
import time
from ctypes import *

class playokBot():
	def __init__(self,driver):
		self.driver=driver
		self.driver.get("https://www.playok.com/es/damas")

	def login(self):
		# Logeado
		self.driver.find_element_by_xpath("//button[@class='lbpbg']").click()	# Iniciar sesion
		usuario = self.driver.find_element_by_name("username")
		contrasena = self.driver.find_element_by_name("pw")
		usuario.send_keys("AureClaudi")
		contrasena.send_keys("MatesUAB")
		self.driver.find_element_by_xpath("//input[@class='bxpad ttup']").click()	# Click para login

		# Empezar damas
		self.driver.find_element_by_xpath("//button[@class='lbprm']").click()	# Entrar en seleccion de tablero
		self.driver.switch_to.window(driver.window_handles[1])

	def nueva_mesa(self):
		# Esperar hasta que aparezca el boton de crear mesa y clicar
		driver.implicitly_wait(5)
		driver.find_element_by_xpath("//button[@class='butsys minwd']").click() # Crear mesa
		driver.implicitly_wait(1)
		driver.find_element_by_xpath("//button[@class='butsys butsit']").click()	# Seleccionar Blancas

	def empezar(self):
		self.driver.find_elements_by_xpath("//button[@class='butwb']")[1].click() # Empezar
	
	def tomar_medidas(self,pulsar=True):
		# Veo en que posicion juega el bot
		# Jugador 1
		self.jugador1_nombre=self.driver.execute_script("return document.getElementsByClassName('tplcont')[0].children[0].children[1].children[1].children[1].textContent")
		self.jugador1_color=None
		color=self.driver.execute_script("return document.getElementsByClassName('tplcont')[0].children[0].children[0].children[0].style['background']")	
		if color=='rgb(255, 17, 17)':
			self.jugador1_color='rojo'
		elif color=='rgb(255, 255, 255)':
			self.jugador1_color='blanco'
		else: print("Color del jugador 1 no identificado:",color)
		# Jugador 2
		self.jugador2_nombre=self.driver.execute_script("return document.getElementsByClassName('tplcont')[0].children[1].children[1].children[1].children[1].textContent")
		self.jugador2_color=None
		color=self.driver.execute_script("return document.getElementsByClassName('tplcont')[0].children[1].children[0].children[0].style['background']")
		if color=='rgb(255, 17, 17)':
			self.jugador2_color='rojo'
		elif color=='rgb(255, 255, 255)':
			self.jugador2_color='blanco'
		else: print("Color del jugador 2 no identificado:",color)
		# Punto de vista		
		print("jugador1:",self.jugador1_nombre,"jugador2:",self.jugador2_nombre)
		self.punto_de_vista=0
		if self.jugador1_nombre=='aureclaudi':
			if self.jugador1_color=='blanco':
				self.punto_de_vista=1
				print("Juegas con blancas")
			elif self.jugador1_color=='rojo':
				self.punto_de_vista=3
				print("Juegas con rojas")
			else: print("Punto de vista no identificado")
			
		elif self.jugador2_nombre=='aureclaudi':
			if self.jugador2_color=='blanco':
				self.punto_de_vista=1
				print("Juegas con blancas")
			elif self.jugador2_color=='rojo':
				self.punto_de_vista=3
				print("Juegas con rojas")
			else: print("Punto de vista no identificado")
		else: print("No juegas en esta partida")

		# Guardo las dimensiones y coordendas de la ficha que esta abajo de todo a la izquierda
		self.tablero_selenium=self.driver.find_element_by_tag_name('canvas')
		self.selenium_damas=self.driver.find_elements_by_tag_name('img')#[:24]# el [:24] es porque algunas veces hay 25 imagenes y una tiene location=(0,0) y esta resulta ser la ultima		
		self.longitud=int(self.selenium_damas[0].get_attribute('width'))
		x=min([d.location['x'] for d in self.selenium_damas if d.location['x']!=0])
		y=max([d.location['y'] for d in self.selenium_damas ])
		self.a1=[x,y]
		if pulsar==True:
			self.driver.execute_script(open("./playok.js").read())
			self.js_poner_svg()
			self.js_encuadrar()
			input("Toca cualquier tecla para continuar")
			self.js_quitar_svg()

	def js_poner_svg(self):
		self.driver.execute_script('poner_svg()')

	def js_quitar_svg(self):
		self.driver.execute_script('quitar_svg()')

	def js_encuadrar(self):
		o=self.casilla_a_offset([0,0])
		self.driver.execute_script('encuadrar('+str(o[0])+','+str(o[1])+','+str(8*self.longitud)+')')

	# Esta funcion de aqui abajo ya no sirve para nada
	def js_malla(self):
		# Para que la malla se encuadre bien tengo que borrar un elemento que molesta
		self.padre=self.driver.execute_script("return document.getElementsByClassName('tsinbo bsbb')[0]")
		self.cosa=self.driver.execute_script("return arguments[0].children[0]",self.padre)
		self.cosa_html=self.cosa.get_attribute('innerHTML') # esto es una string
		self.driver.execute_script("arguments[0].remove()",self.cosa)
		self.js_poner_svg()
		self.js_encuadrar()
		input("Toca cualquier tecla para continuar")
		self.js_quitar_svg()
		#self.driver.execute_script("arguments[0].append(arguments[1])",self.padre,self.cosa)
		self.driver.execute_script("arguments[0].innerHTML=arguments[1]",self.padre,self.cosa_html)

	def casilla_a_offset(self,casilla,punto_de_vista=1):
		if punto_de_vista==3:
			casilla=[7-casilla[0],7-casilla[1]]
		return [self.a1[0]+casilla[0]*self.longitud,self.a1[1]-(7-casilla[1])*self.longitud]

	def click_casilla_offset(self,casilla):
		webdriver.common.action_chains.ActionChains(self.driver).move_to_element_with_offset(self.tablero_selenium,casilla[0],casilla[1]).click().perform()

	def mover(self,origen,destino):
		self.click_casilla_offset(self.casilla_a_offset(origen,self.punto_de_vista))
		time.sleep(0.1)
		self.click_casilla_offset(self.casilla_a_offset(destino,self.punto_de_vista))

	def leer_tablero(self,profundidad=12):
		self.tomar_medidas(False)
		self.inicializar_motor(profundidad)
		# Modifico la inicializacion
		self.turno=self.punto_de_vista
		self.engine_plays_as=c_int(self.punto_de_vista)
		#tbody=self.driver.find_element_by_class_name('br').find_element_by_tag_name('tbody')
		#self.ply=2*len(tbody.find_elements_by_tag_name('tr'))-2
		movimientos_anteriores=self.driver.find_elements_by_xpath('//td[@class=""]')
		self.ply=len(movimientos_anteriores)+1
		if (self.punto_de_vista==1 and self.jugador2_nombre=='aureclaudi') or (self.punto_de_vista==3 and self.jugador1_nombre=='aureclaudi'):
			self.ply=self.ply-1
		self.tablero=(c_int*8*8)(
			(0,0,0,0,0,0,0,0),
			(0,0,0,0,0,0,0,0),
			(0,0,0,0,0,0,0,0),
			(0,0,0,0,0,0,0,0),
			(0,0,0,0,0,0,0,0),
			(0,0,0,0,0,0,0,0),
			(0,0,0,0,0,0,0,0),
			(0,0,0,0,0,0,0,0),
		)
		letras=['a','b','c','d','e','f','g','h']
		tablero=[[0 for k in range(8)] for i in range(8)]
		for d in self.selenium_damas:
			l=[d.location['x'],d.location['y']]
			if l[0]==0 or l[1]==0 : continue
			casilla=[int((l[0]-self.a1[0])/self.longitud),7-int((self.a1[1]-l[1])/self.longitud)]
			if self.turno==3: casilla=[7-casilla[0],7-casilla[1]]
			#print(casilla)
			s_casilla=letras[casilla[0]]+str(7-casilla[1]+1)
			#print(casilla)
			# Ahora voy a ver que tipo de pieza es:
			# Para ello las diferenciare usando la longitud del src de su imagen
			dict={2958:1,3006:2,3146:3,3178:4}
			pieza=dict[len(d.get_attribute('src'))]
			print("Hay un",pieza,"en",s_casilla)
			tablero[casilla[1]][casilla[0]]=pieza
			self.tablero[casilla[1]][casilla[0]]=pieza
		for f in tablero: print(f)
			
			
	def leer_movimiento_contrario(self):
		#reloj=self.driver.find_element_by_xpath('/div/div[1]/div[3]/div/div')
		#style=reloj.get_attribute('style')
		if self.jugador1_nombre=='aureclaudi': indice='6'
		elif self.jugador2_nombre=='aureclaudi': indice='15'
		reloj=self.driver.execute_script("return document.getElementsByClassName('tplcont')[0].getElementsByTagName('div')["+indice+"].textContent")
		time.sleep(0.2) # Por si acaso
		while True:
			t_reloj=self.driver.execute_script("return document.getElementsByClassName('tplcont')[0].getElementsByTagName('div')["+indice+"].textContent")
			if reloj!=t_reloj:
				print("Ya es mi turno: ha pasado de "+reloj+' a '+t_reloj)
				break
			t_texto=self.driver.find_elements_by_class_name('tstatlabl')[1].text
			if t_texto!='':
				# Quiere decir que el texto de este elemento pone algo asi como: el jugador #1 ha ganado y por lo tanto ha cabado l apartida
				self.fin_de_partida=True
				print("leer_movimiento_contrario(): La partida ha finalizado!")
				return 0
			print("Esperando movimiento contrario... reloj:",t_reloj,"texto:",t_texto,end='\r')
			time.sleep(0.1)
		#print("Mi reloj ha pasado de",reloj,"a",t_reloj,"por lo tanto el oponente ya ha movido")
		time.sleep(0.1)
		barra_menu=self.driver.find_elements_by_xpath('//div[@class="tcrdcell"]')
		#print(len(barra_menu))
		barra_menu[1].click()
		movimientos_anteriores=self.driver.find_elements_by_xpath('//td[@class=""]')
		print("Hay",len(movimientos_anteriores),"movimientos anteriores y vamos por el ply=",self.ply)
		#while(len(movimientos_anteriores)%2==0):
		extra=0
		if (self.punto_de_vista==1 and self.jugador2_nombre=='aureclaudi') or (self.punto_de_vista==3 and self.jugador1_nombre=='aureclaudi'):
			extra=1
		while(len(movimientos_anteriores)<self.ply+extra):
			movimientos_anteriores=self.driver.find_elements_by_xpath('//td[@class=""]')
			print("Esperando movimiento contrario...",len(movimientos_anteriores),"<",self.ply,"+",extra,end='\r')
			t_texto=self.driver.find_elements_by_class_name('tstatlabl')[1].text
			if t_texto!='':
				# Quiere decir que el texto de este elemento pone algo asi como: el jugador #1 ha ganado y por lo tanto ha cabado l apartida
				self.fin_de_partida=True
				print("leer_movimiento_contrario(): La partida ha finalizado!")
				return 0
			time.sleep(0.1)
		ultimo_movimiento=self.driver.find_element_by_xpath('//td[@class="fb"]').text
		print("El adversario ha movido:",ultimo_movimiento)
		return ultimo_movimiento


	def inicializar_motor(self,profundidad=12):
		self.libDamas = CDLL("./damas.so")
		n = c_int(8)
		self.depth = c_int(profundidad)
		self.engine_plays_as = c_int(self.punto_de_vista)
		self.show_best_move = c_int(1)
		self.show_engine_process = c_int(0)
		max_depth_showing = c_int(1)
		MAX_MOV = c_int(30) # quizas no hace falta el c_int
		class mov(Structure):
			_fields_ = [
				("casilla_destino", (c_int*2)),
				("casilla_origen", (c_int*2)),
				("mata", c_int),
				("casilla_victima", (c_int*2)),
				("valor", c_long)]

		self.libDamas.computadora.restype=mov
		self.libDamas.computadora_sinpoda.restype=mov
		self.libDamas.computadora_conpoda.restype=mov

		self.tablero = (c_int*8*8)()
		self.turno = 1
		self.libDamas.inicio_de_partida(self.tablero)
		self.movimiento_pedido = mov()
		self.MEMORIA_PARTIDA = (c_int*8*8*200)()
		self.ply = 0
		self.siguientes_movimientos = (c_char*6*3)()
		self.cantidad_de_siguientes_movimientos = c_int(0)
		self.casilla_del_atacante = (c_int*2)()
		self.casilla_del_atacante[0] = c_int(99)
		self.casilla_del_atacante[1] = c_int(99)
		self.posibles_movimientos=(mov*MAX_MOV.value)()
		self.movimientos_PDN = (c_char*20*250)()
		#libDamas.imprimir_tablero(self.tablero)
		self.mejor_movimiento = mov()
		self.acaba_de_coronar = c_int(0)
		self.fin_de_partida=False # Para detectar si se ha acabadi a partida por tiempo o el otro se ha rendido
		print("Inicializado: punto_de_vista:",self.punto_de_vista,"profundidad:",self.depth)

	def activar_motor(self,manual=False,timeset=2):
		# Bucle del juego
		valido = 0
		limite = 0
		movimiento_en_dos_tiempos=False # Esta varible indica cuando el contrario come mas de una vez en cadena y lo hace clicando en dos tiempos
		while self.libDamas.sin_fichas(self.tablero, self.turno) == 0 and self.libDamas.generar_movimientos(self.tablero, self.turno, self.posibles_movimientos, self.casilla_del_atacante,0)>0 and not self.fin_de_partida:
			if self.show_best_move.value == 1 and self.turno == self.engine_plays_as.value:
				#self.mejor_movimiento = self.libDamas.computadora(self.tablero, self.turno, self.casilla_del_atacante, self.depth, self.show_engine_process,1,timeset)
				t1=time.time()
				self.mejor_movimiento = self.libDamas.computadora_conpoda(self.tablero, self.turno, self.casilla_del_atacante, self.depth, self.show_engine_process)
				if time.time()-t1>30: self.depth=8;print("python: Bajo la profundidad a 8")
				#elif time.time-t1<0.1: self.depth=self.depth+1;print("python: Aumento la profundidad a",self.profundidad)
				#elif time.time-t1>10: self.depth=self.depth-1;print("python: Bajo la profundidad a",self.profundidad)
				"""print("calculando sin poda...")
				m2=self.libDamas.computadora_sinpoda(self.tablero, self.turno, self.casilla_del_atacante, self.depth, self.show_engine_process)
				print("comparando...")
				if self.libDamas.comparador_movimientos(self.mejor_movimiento,m2)==0:
					print("Los movimientos no han sido los mismos!")
					#print(m2.casilla_origen,m2.casilla_destino,m2.valor)
				else: print("De momento todo ok")"""
			if self.turno == self.engine_plays_as.value:
				print("me toca: muevo de ",list(self.mejor_movimiento.casilla_origen),"a",list(self.mejor_movimiento.casilla_destino))
				if manual==False: self.movimiento_pedido=self.mejor_movimiento
				#else: self.movimiento_pedido=input('Introduce un movimiento: ')
				self.acaba_de_coronar = c_int(0)
				valido = self.libDamas.moivmiento_valido(self.tablero, self.mejor_movimiento, self.turno, self.casilla_del_atacante, byref(self.acaba_de_coronar))
				# FUNCION MOVER FICHA DEL SELENIUM AQUI ABAJO
				# LOS MOVIMIENTOS DEL MOTOR DE DAMAS SON DE LA FORMA [Y,X]
				# Y LOS MOVIMIENTOS DEL PYTHON SON DE LA FORMA [X,Y]
				# AMBOS CON EL MISMO CONVENIO DE TOP LEFT CON EL [0,0] ARRIBA A LA IZQUIERDA
				time.sleep(0.5)
				self.mover([self.mejor_movimiento.casilla_origen[1],self.mejor_movimiento.casilla_origen[0]],[self.mejor_movimiento.casilla_destino[1],self.mejor_movimiento.casilla_destino[0]])
			else:
				# FUNCION LEER MOVIMIENTO CONTRARIO SELENIUM AQUI
				#print("Espero 0.5 seg antes de empezar a mirar que ha movido")
				#time.sleep(0.5)
				#input('pulsa cualquier tecla para continuar')
				if self.cantidad_de_siguientes_movimientos.value==0:
					movimiento_contrario=self.leer_movimiento_contrario()
					if movimiento_contrario==0: return 0 # Significa que se ha acabado la partida
					if movimiento_en_dos_tiempos==True:
						# Significa que no se ha cambiado de turno porque el contrario va a volver a comer.
						# Lo que sucede es que primero se la ha leido el movimiento e3x5 y ahora al mover
						# por segunda vez se ha leido el movimiento e3xc5xe7
						# Podria darse el caso en el que la combinacion sea muy larga.
						# De momento hare una chapuza y borrare el e3x
						movimiento_contrario=movimiento_contrario[3:]
				print("python:\n\tmovimiento_leido:",movimiento_contrario,"\n\tlen=",len(movimiento_contrario),"\n\tcantidad_de_siguientes_movimientos:",self.cantidad_de_siguientes_movimientos.value)				
				self.libDamas.pedir_movimiento_selenium(byref(self.movimiento_pedido), self.turno, self.ply, self.movimientos_PDN, self.siguientes_movimientos, byref(self.cantidad_de_siguientes_movimientos), c_char_p(movimiento_contrario.encode('utf-8')))
				self.acaba_de_coronar = c_int(0);
				valido = self.libDamas.moivmiento_valido(self.tablero, self.movimiento_pedido, self.turno, self.casilla_del_atacante, byref(self.acaba_de_coronar))	# Convertir a int?
				if valido == 0:
					print("DAMAS: ERROR: algo raro ha sucedido al pedir un movimiento valido\n")
					return 0
			# Imprimo tablero
			self.libDamas.imprimir_tablero(self.tablero)
			valido = 0
			limite = 0
			# Guardo partida
			self.libDamas.guardar_partida(self.tablero, self.MEMORIA_PARTIDA[self.ply])
			# Cambio de turno adecuado
			if self.libDamas.puede_matar(self.tablero, self.turno, self.movimiento_pedido.casilla_destino, c_int(0)) == 1 and self.acaba_de_coronar.value == 0 and self.movimiento_pedido.mata == 1:
				self.casilla_del_atacante[0] = self.movimiento_pedido.casilla_destino[0]
				self.casilla_del_atacante[1] = self.movimiento_pedido.casilla_destino[1]
				movimiento_en_dos_tiempos=True
				continue
			else:
				self.casilla_del_atacante[0] = c_int(99)
				self.casilla_del_atacante[1] = c_int(99)
				movimiento_en_dos_tiempos=False
				if self.turno == 1:
					self.turno = 3
				else:
					self.turno = 1
				self.ply = self.ply+1

		if self.turno == 1:
			self.turno = 3
		else:
			self.turno = 1
		#libDamas.system("clear")
		self.libDamas.imprimir_tablero(self.tablero)
		#print("Se acabo la partida")

	def run(self,profundidad=12,timeset=2):
		self.tomar_medidas(pulsar=False)
		self.inicializar_motor(profundidad)
		self.activar_motor(timeset=2)


def clear():
	os.system("clear")

if __name__=='__main__':
	driver=webdriver.Chrome('/home/tete/Escritorio/aure/chromedriver_linux64/chromedriver')
	b=playokBot(driver)
	b.login()
	#b.nueva_mesa()
# Siguie fallando que no lee bien cuando le comen dos veces. (solo a veces y no se my bien porque.)



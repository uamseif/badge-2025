# 

Este proyecto combina hardware y software en una placa PCB interactiva que permite jugar a tres minijuegos clásicos: **Ping Pong**, **Simon Dice** y **Reflejos**.  
La placa está equipada con botones, LEDs y la capacidad de conectarse con otra para jugar en **modo multijugador**.

A continuación, encontrarás las instrucciones para navegar por el menú principal, seleccionar juegos y comprender la dinámica de cada uno, tanto en modo individual como multijugador.

## Menú Principal

- **Encender la placa**
  Para encender la placa, basca con mover el switch SW7 a la posición de encendido. Acto seguido, la placa entra en modo *salvapantallas*.

- **Salir del modo salvapantallas:**  
  Pulsa cualquiera de los botones **A**, **B**, **C** o **D** para salir del salvapantallas y acceder al menú principal.

- **Indicador visual de selección:**  
  Cuatro LEDs (D1, D2, D3 Y D4) en la placa mostrarán la selección actual en formato binario, facilitando saber qué juego está seleccionado.

- **Seleccionar el juego:**  
  Utiliza los botones **C** y **D** para moverte entre los minijuegos disponibles:
  
  | Juego                                             | D1 D2 D3 D4 |
  | ------------------------------------------------- | ----------- |
  | [Ping Pong](#ping-pong)                           | 0001        |
  | [Simon Dice](#simon-dice)                         | 0010        |
  | [Reflejos](#reflejos)                             | 0011        |
  | [Ping Pong Multijugador](#ping-pong-multijugador) | 0100        |
  | [Reflejos Multijugador](#reflejos-multijugador)   | 0101        |

- **Confirmar selección:**  
  Presiona el botón **B** para iniciar el juego seleccionado.

- **Volver al menú:**
  Pulsa el botón MENU para volver al menú principal desde cualquier juego. 

> :memo: **Nota**:
> 
> - Para jugar en modo multijugador, **dos placas deben conectarse** físicamente entre sí tal como se muestra en la foto 
>   <img title="" src="images/multi.jpg" alt="" width="600">.
> - Uno de los jugadores debe iniciar un juego en **modo multijugador** desde el menú principal para comenzar la partida entre ambas placas.

## Ping Pong

En el juego Ping Pong, la mecánica es sencilla pero requiere buen timing y concentración.

- **Inicio del juego:**  
  Al comenzar, una pelota representada por un LED caerá por la línea central de LEDs.

- **Objetivo:**  
  Debes hacer rebotar la pelota todo el tiempo que puedas, pero recuerda que no puedes dejar que caiga o hacerla rebotar antes de tiempo

- **Cómo hacer rebotar la pelota:**  
  Para hacerla rebotar, pulsa cualquiera de los botones **D** o **B**.  
  **Importante:** La pelota solo rebotará si está encendida en los LEDs naranjas, que corresponden a **D14** y **D15**.

## Simon Dice

Simon Dice es un juego clásico de memoria y atención.

- **Controles y colores:**  
  La placa tiene cuatro grupos de colores, cada uno asociado a un botón:
  
  - **Rojo** — botón **C**
  - **Verde** — botón **D**
  - **Amarillo** — botón **A**
  - **Azul** — botón **B**

- **Dinámica del juego:**  
  La placa iniciará mostrando una secuencia de LEDs encendiéndose en distintos colores.  
  Tu tarea es **replicar la secuencia** pulsando los botones correspondientes en el mismo orden.

- **Progresión:**  
  Cada vez que completes correctamente la secuencia, el juego añadirá un color nuevo al final de la lista y deberás recordarla y repetirla con la nueva adición.

- **Objetivo:**  
  Mantén la concentración y recuerda la secuencia creciente para seguir avanzando y lograr la mayor puntuación posible.

## Reflejos

Reflejos es un juego que pone a prueba tu rapidez y tiempo de reacción.

- **Controles y colores:**  
  Igual que en Simon Dice, cada color está asociado a un botón:
  
  - **Rojo** — botón **C**
  - **Verde** — botón **D**
  - **Amarillo** — botón **A**
  - **Azul** — botón **B**

- **Dinámica del juego:**  
  La placa mostrará un color iluminando el LED correspondiente.  
  Tu objetivo es **pulsar rápidamente el botón asociado a ese color**.

- **Dificultad creciente:**  
  El tiempo para reaccionar irá disminuyendo progresivamente, haciendo que debas estar cada vez más atento y rápido para no perder.

- **Objetivo:**  
  Mantén la concentración y responde rápido para lograr la máxima puntuación posible.

## Ping Pong Multijugador

El modo multijugador de Ping Pong mantiene la misma mecánica que el modo normal, pero con dos jugadores enfrentados.

- **Dinámica:**  
  La pelota rebota entre los dos usuarios, que deberán hacerla rebotar correctamente usando los botones correspondientes.

- **Objetivo:**  
  Cada jugador debe hacer rebotar la pelota en el momento justo.  
  Pierde el jugador que haga rebotar la pelota fuera de tiempo o que deje que caiga sin rebotar.

## Reflejos Multijugador

El modo multijugador de Reflejos se basa en turnos entre dos jugadores.

- **Dinámica:**  
  El jugador que inicia el juego genera un color (LED encendido), y el otro jugador debe pulsar rápidamente el botón correspondiente.  
  Luego, el turno cambia y el siguiente jugador genera un color para que el contrario responda.

- **Objetivo:**  
  Cada jugador debe reaccionar rápido y correctamente al color mostrado por el oponente.  
  El juego continúa alternando turnos, poniendo a prueba los reflejos de ambos jugadores.

## Marcador Final

Al finalizar cualquier juego, la placa muestra un marcador visual que indica tu rendimiento en cada minijuego. Este marcador utiliza los LEDs como contador de aciertos. Si el número de aciertos supera los 10, el contador se reinicia, repitiendo el ciclo hasta llegar al número de aciertos.

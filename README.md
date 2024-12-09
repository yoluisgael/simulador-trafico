# simulador-trafico
Este proyecto es un simulador de tráfico desarrollado utilizando la Regla 184 de los Autómatas Celulares Elementales (ECA). La simulación emplea vectores para representar y actualizar las posiciones de los vehículos, modelando dinámicas de tráfico en escenarios controlados.

El simulador tiene como objetivo estudiar el comportamiento del tráfico vehicular bajo ciertas condiciones, utilizando la Regla 184, conocida por ser útil en la modelización de sistemas de flujo unidimensionales como el tráfico en carreteras.

Permite ver ciertos comportamientos como:
* Flujo constante de tráfico.
* Formación de congestionamientos y su disipación.
* Efectos de densidad vehicular en la dinámica del sistema.

Video de youtube:

[![Simulador de Tráfico](https://img.youtube.com/vi/MlvBdhOyccU/0.jpg)](https://www.youtube.com/watch?v=MlvBdhOyccU)


## Características
* Modelo matemático basado en autómatas celulares: Regla 184.
* Representación visual: Simulación de tráfico en tiempo real.
* Control paramétrico: Ajusta la densidad inicial de vehículos, la velocidad máxima y la longitud del carril.
* Resultados escalables: Puede adaptarse a diferentes longitudes de carreteras o densidades vehiculares.

## Cómo usar

1.  Clona el repositorio.
2.  Compila el código.
3.  Ejecuta el programa.
4.  Modifica los parámetros a tu gusto (opcional).
5.  Comienza el simulador.
6.  Observa el flujo del tráfico.

## Controles

*   Clic izquierdo: Dibujar autos en las calles.
*   Tecla Espacio: Reanudar/Pausar simulador.
*   Tecla Esc: Salir del programa.

## Parámetros

*   Speed: Determina la velocidad de ejecución.
*   Stop: Indica la probabilidad de que un carro se detenga cerca de la entrada (Cuadro negro).
*   Park: Indica la probabilidad de que un carro se estacione al encontrar un lugar vacío.
*   Turn: Indica la probabilidad de dar vuelta a otra calle.
*   Spawn: Indica la probabilidad de que entre un carro cada paso.

## Dependencias

*   SFML

## Contribuciones

Las contribuciones son bienvenidas. Crea un pull request con tus cambios.

## Licencia

Este proyecto está licenciado bajo la Licencia MIT.

[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/XglgMq0o)
# Documentación del Proyecto
---
Estudiante:  
Id:  
---
1. ¿Cuál es la diferencia entre un programa y un proceso?
Un programa es solo un archivo con instrucciones (como un archivo .exe).
Un proceso es cuando el programa está corriendo en la computadora, usando memoria y CPU.
2. ¿Puedo tener múltiples procesos corriendo el mismo programa?
Sí, puedes tener muchos procesos ejecutando el mismo programa al mismo tiempo. Ejemplo: abrir varias ventanas del navegador.

3. ¿Para qué sirve el stack de un proceso?
El stack (pila) guarda:

Variables temporales y datos de funciones.
Direcciones para volver al punto correcto cuando una función termina.
4. ¿Para qué sirve el heap de un proceso?
El heap (montículo) es para guardar datos que necesitas en el programa por más tiempo y que pueden cambiar de tamaño. Usas funciones como malloc() para pedir memoria y free() para liberarla.

5. ¿Qué es la zona de texto de un proceso?
Es donde se guarda el código que ejecuta tu programa (las instrucciones). Está protegido para que no se modifique mientras el programa corre.

6. ¿Dónde se almacenan las variables globales inicializadas?
Las variables globales que tienen un valor asignado desde el inicio se guardan en la sección de datos.

7. ¿Dónde se almacenan las variables globales no inicializadas?
Las variables globales que no tienen un valor asignado al inicio se guardan en la sección BSS y se inicializan a cero automáticamente.

8. ¿Cuáles son los posibles estados de un proceso en general?
Los estados comunes de un proceso son:

Nuevo: Se está creando.
Listo: Esperando su turno para usar la CPU.
Ejecutando: Usando la CPU.
Bloqueado: Esperando algo (como entrada/salida).
Terminado: Ha terminado y está listo para ser eliminado.

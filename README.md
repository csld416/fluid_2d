# Credits

Inspired by HirschDaniel, I saw his project and decide to try pn myself.

# Logs

## 2024/12/25

Started this project. I thought since my major
is related to ocean engineering, and i love code
in C at the same time, simulating the fluid in
pure C might just be a brand new experience for
me.

Today, I learned about SDL2, which is the infrastruccture that I will be using for the entire project.

Later, I created the grid for the entire simulation where each cell is a pixel of the simulation.

Added the color definition for the solid cells and the background cells, as well as the water cells.

## 2024/12/28

Took 2 days off (actually, going out with my gf). Anyways, for a liquid simulation there should be a slod container or smething that should be at the users disposal.

Declared a 1d array as `environment`for storing the information of each cell of the 2d grid. using the 1d array is for easy manipulation (Really?)

learned how to draw with SDL2, enabled the drawing method of this simulation.

## 2024/12/31 (Happy new year)

Took some days off(I forgot what I did)

Added: let the user to switch the drawing mode between solid and liquid.

## 2025/01/02

(Dealing with computer architecture for the opast two days)

Added: flowing mechanism, the water flows down automatically. Moreover, the water stays from falling or disappearing if the below cell is a solid block.

## 2025/01/03

Bug fix: When drawing in water mode, it doesn't overwrite the solid cells.

Added: Left flow of the water and right flow of the water

## 2025/01/06

Added: the distingiuishment of the water based on the pressure of each cell.

By assgining light blue and dark blue, define the maximum current pressure to be the dark blue, and the minimu to be zero. Later, we use interpolation to generate the color for each cell.

## 2025/01/07

Added(big fix): the ability to add water cells below the fluid surface.

### To Be Done:

- Viscocity
- Pressure of the water
- Color of the cell with respect to the pressure

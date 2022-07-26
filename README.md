# DoubleFaceText

Convert *Text*, Images, and NBT into Mesh or into each other

## Build
```
make
```

## Usage
```
./dft [INPUT] FROM [INPUT TYPE] TO [OUTPUT TYPE] AS [OUTPUT PATH]
```

### INPUT
Path to input file/s.
1-3 are possible

#### Example
```
./dft structure.nbt FROM NBT TO MESH AS output.stl
```
This will convert the NBT voxel structure to a STL-Mesh and save it as "output.stl"

### INPUT TYPE

Type of Input
#### Supported types
- IMG
- NBT
- *TEXT*

#### Example
```
./dft TextA.png TextB.bmp FROM IMG IMG TO MESH AS output.obj
```
Will interpret Images "TextA.png" and "TextB.bmp" as shadows of a 3D-Object and output a corresponding Wavefront-OBJ Mesh to "output.obj"


![Viz](https://user-images.githubusercontent.com/69124976/181050549-83ff47a5-f485-4ffb-b0ea-14c00f916170.png)

### OUTPUT TYPE
Output types
multiple conversion are allowed at once
#### Supported types
- SHADOW
- MESH
- NBT

#### Example
```
./dft input.nbt FROM NBT TO MESH SHADOW AS output.obj out.jpg
```
Takes the NBT-Voxel Volume and creates both a Wavefront-OBJ Mesh and the corresponding shadow approximation

### OUTPUT
Output path/s

for otput of type Shadow the file name will be suffixed with \_front, \_side, and \_top
The above example will therefore create the files "output.obj", "out_front.jpg", "out_side.jpg", and "out_top.jpg"

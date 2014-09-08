# ZopfliDll

ZopfliDll is a custom extension library for Microsoft IIS that can compress with [Google's Zopfli](https://code.google.com/p/zopfli/) algorithm or a configurable command line tool (such as [7-Zip](http://www.7-zip.org/)). 
It can replace the builtin [gzip compression library](http://msdn.microsoft.com/en-us/library/dd692872.aspx) (gzip.dll).

## Usage (Zopfli)

1. Copy the `ZopfliDll.dll` (or `ZopfliDll64.dll`) file to a folder accessible by the IIS process. 
2. Edit the `<scheme>` element of the `<httpCompression>` element in [`applicationHost.config`](http://www.iis.net/learn/get-started/planning-your-iis-architecture/introduction-to-applicationhostconfig) (unfortunately, these settings cannot be overriden in Web.config):
```xml
<httpCompression>
   <scheme name="gzip" dll="Path\To\ZopfliDll64.dll" />
</httpCompression>
```
3. Restart the "World Wide Web Publishing Service".

Because you cannot have different DLLs for dynamic and static compression (and the DLL does not know whether it's compressing dynamic or static content), ZopfliDll uses the compression level to switch between the builtin fast gzip compression and the slow Zopfli algorithm:

<table>
<tr><th>Compression Level in IIS config</th><th>Compression level used</th></tr>
<tr><td colspan="2" align="center">IIS builtin</td></tr>
<tr><td>0</td><td>0</td></tr>
<tr><td>1</td><td>2</td></tr>
<tr><td>2</td><td>4</td></tr>
<tr><td>3</td><td>6</td></tr>
<tr><td>4</td><td>8</td></tr>
<tr><td>5</td><td>10</td></tr>
<tr><td colspan="2" align="center">Zopfli (iterations)</td></tr>
<tr><td>6</td><td>1</td></tr>
<tr><td>7</td><td>5</td></tr>
<tr><td>8</td><td>10</td></tr>
<tr><td>9</td><td>15</td></tr>
<tr><td>10</td><td>20</td></tr>
</table>

## Usage (command line tool)

ZopfliDll has a second mode of operation which allows you to compress content using an arbitrary command line tool. If ZopfliDll finds a file called `cmd.txt` in the same folder as the DLL, it uses the content of this file to start a command line tool for each stream of data to compress. 

The command line tool should read uncompressed data from stdin and write compressed data in gzip format to stdout. The command line may contain environment variables and a placeholder for the compression level. Example:

```
%ProgramFiles%\7-Zip\7z.exe a dummy -tgzip -mx={1;9} -si -so
```

Compression levels of 0-5 in the IIS configuration are used for compression with the builtin gzip compression as in the table above. Levels 6-10 are spread to the range in the placeholder (1-9 in the example).

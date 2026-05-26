param(
    [string]$FontFile,
    [string]$OutputHeader,
    [string]$VarName
)

$bytes = [System.IO.File]::ReadAllBytes($FontFile)
$len = $bytes.Length
$sb = [System.Text.StringBuilder]::new()

[void]$sb.AppendLine("// Auto-generated embedded font data - do not edit")
[void]$sb.AppendLine("#pragma once")
[void]$sb.AppendLine("#include <cstddef>")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("static const unsigned char ${VarName}_data[] = {")

for ($i = 0; $i -lt $len; $i++) {
    if ($i % 16 -eq 0) { [void]$sb.Append("    ") }
    [void]$sb.Append("0x{0:X2}," -f $bytes[$i])
    if (($i + 1) % 16 -eq 0 -or $i -eq $len - 1) { [void]$sb.AppendLine("") }
}

[void]$sb.AppendLine("};")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("static const unsigned int ${VarName}_size = ${len}u;")

[System.IO.File]::WriteAllText($OutputHeader, $sb.ToString(), [System.Text.Encoding]::ASCII)
Write-Host "Font embedded: $FontFile -> $OutputHeader ($len bytes)"

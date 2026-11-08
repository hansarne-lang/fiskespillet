# Helpers for Run-All.ps1, kept separate so Test-RunAllLib.ps1 can test them.

# True when an err.txt holds anything other than PowerShell progress records.
# Windows PowerShell can write progress ("Preparing modules for first use") to stderr as
# CLIXML. Only that is ignored. Fail-closed: any other text, any top-level CLIXML element that
# is not an <Obj S="progress">, an empty CLIXML document or CLIXML that cannot be parsed counts
# as an error.
function Test-ErrorOutput([string]$path) {
    if (-not (Test-Path -LiteralPath $path)) { return $false }
    $text = Get-Content -LiteralPath $path -Raw
    if ([string]::IsNullOrWhiteSpace($text)) { return $false }
    return Test-ErrorText $text
}

function Test-ErrorText([string]$text) {
    if ([string]::IsNullOrWhiteSpace($text)) { return $false }
    $parts = $text.TrimStart() -split "`r?`n", 2
    if ($parts[0].Trim() -ne '#< CLIXML' -or $parts.Count -lt 2 -or [string]::IsNullOrWhiteSpace($parts[1])) { return $true }
    try {
        $xml = New-Object System.Xml.XmlDocument
        $xml.LoadXml($parts[1])
    } catch {
        return $true
    }
    $root = $xml.DocumentElement
    if ($null -eq $root -or $root.LocalName -ne 'Objs') { return $true }
    $seen = 0
    foreach ($node in $root.ChildNodes) {
        switch ($node.NodeType) {
            'Whitespace' { continue }
            'SignificantWhitespace' { continue }
            'Comment' { continue }
            'Element' {
                if ($node.LocalName -ne 'Obj' -or $node.GetAttribute('S') -ne 'progress') { return $true }
                $seen++
            }
            default { return $true }
        }
    }
    return $seen -eq 0
}

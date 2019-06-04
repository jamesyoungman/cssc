BEGIN {
    copyright = ""
    while ((getline line < text) > 0)
    {
	copyright = copyright "\"" line "\\n\"\n"
    }
    close(text)
}

/copyright[.]cc[.]tmpl/ {
    gsub("copyright[.]cc[.]tmpl",
	 outputfile " (generated from copyright_data.inc.tmpl)");
}

/___COPYRIGHT_TEXT___/ {
    gsub("___COPYRIGHT_TEXT___", copyright)
}

{
    print;
}

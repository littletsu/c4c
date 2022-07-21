#ifndef HTML_H
#define HTML_H

#include <stdio.h>
#include <string.h>
#include <tidy.h>
#include <tidybuffio.h>


char *htmlInnerText(ctmbstr html) {
	TidyDoc tdoc = tidyCreate();
	tidyOptSetValue(tdoc, TidyShowWarnings, "false");
	tidyOptSetValue(tdoc, TidyShowInfo, "false");
	tidyOptSetValue(tdoc, TidyBreakBeforeBR, "false");

	tidySetMessageCallback(tdoc, NULL);
	tidySetPrettyPrinterCallback(tdoc, NULL);
	tidyParseString(tdoc, html);
	TidyNode child;
	char *innerText = (char*)malloc(2000);
	strcpy(innerText, "");
	for(child = tidyGetChild(tidyGetBody(tdoc)); child; child = tidyGetNext(child)) {
		ctmbstr name = tidyNodeGetName(child);
		//printf("%s", name);
		if(name != NULL && (strcmp(name, "br") == 0)) {
			strcat(innerText, "\n");
			continue;
		}
		TidyBuffer buf;
		tidyBufInit(&buf);
		if(tidyNodeGetValue(tdoc, name ? tidyGetChild(child) : child, &buf)) 
			strcat(innerText, (char *)buf.bp);
		tidyBufFree(&buf);

	}
	strcat(innerText, "\0");
	return innerText;
}
#endif

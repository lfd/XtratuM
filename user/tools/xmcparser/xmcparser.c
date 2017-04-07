/*
 * $FILE: xmcparser.c
 *
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "xmcparser.h"
#include "constraints.h"
#include "devices.h"
#include "hm.h"

#define XSD_DEFAULT_FILE0 "xmc.xsd"
#define XSD_DEFAULT_FILE1 "../xsd/xmc.xsd"

#define TOOL_NAME "xmcparser"
#define USAGE "usage: "TOOL_NAME" [-s <xsd_file>] [-o output_file] <XM_CF.xml>"

void ShowErrorMsgAndExit(const char *msg,...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    printf("\n");
    va_end(args);      
    exit(-1);
}

unsigned long TimeStr2Time(char *input) {
    float d;
    sscanf(input, "%f", &d); 
    if(strcasestr(input, "us")) {
    } else if(strcasestr(input, "ms")) {
	d*=1000.0;
    } else  if(strcasestr(input, "s")) {
	d*=1000000.0;
    }

    return (unsigned long)d;
}

unsigned long SizeStr2Size(char *input) {
    float d;
    sscanf(input, "%f", &d);     
    if(strcasestr(input, "mb")) {
	d*=(1024.0*1024.0);
    } else if(strcasestr(input, "kb")) {
	d*=1024.0;
    } /*else  if(strcasestr(input, "b")) {
	} */

    return (unsigned long)d;
}

unsigned long FreqStr2Freq(char *input) {
    float d;
    sscanf(input, "%f", &d);     
    if(strcasestr(input, "mhz")) {
	d*=1000.0;
    } /*else if(strcasestr(input, "khz")) {
	d*=1000.0;
	} */

    return (unsigned long)d;
}

int InsertChildNode(struct nodeXml *root, struct nodeXml *node) {
    int e;
    for (e=0; e<MAX_CHILDREN_PER_NODE; e++) {
	if (!root->children[e]) {
	    root->children[e]=node;
	    if ((e+1)<MAX_CHILDREN_PER_NODE) {
		root->children[e+1]=0;
	    }
	    return 0;
	}
    }
    ShowErrorMsgAndExit("No free child pointer in the node \"%s\"", root->name);
    return -1;
}

struct nodeXml *LookUpNode(struct nodeXml *handlers[], xmlChar *name) {
    struct nodeXml *nodeXml=0;
    int e;
    for (e=0; (e<MAX_CHILDREN_PER_NODE)&&handlers[e]; e++) {
	if (!xmlStrcasecmp(name, handlers[e]->name)) {
	    return handlers[e];
	} else {
	    if ((nodeXml=LookUpNode(handlers[e]->children, name))) 
		return nodeXml;
	}
    }
    return nodeXml;
}

struct attrXml *LookUpAttribute(struct nodeXml *handlers[], xmlChar *node, xmlChar *attr) {
    struct attrXml *attrXml=0;
    int e, i;
    for (e=0; (e<MAX_CHILDREN_PER_NODE)&&handlers[e]; e++) {
	if (!xmlStrcasecmp(node, handlers[e]->name)) {
	    for (i=0; (i<MAX_ATTR_PER_NODE)&&handlers[e]->attrList[i]; i++) {
		if (!xmlStrcasecmp(attr, handlers[e]->attrList[i]->name)) {
		    return handlers[e]->attrList[i];
		}
	    }
	} else {
	    if ((attrXml=LookUpAttribute(handlers[e]->children, node, attr))) return attrXml;
	}
    }
    return attrXml;
}

static void ProcessXmlTree(xmlNodePtr root, struct nodeXml *handlers[]) {
    xmlNodePtr node;
    int e, attr;

    for (node=root; node; node=node->next) {
	if (node->type==XML_ELEMENT_NODE) {
	    for (e=0; (e<MAX_CHILDREN_PER_NODE)&&handlers[e]; e++) {
		if (!xmlStrcasecmp(node->name, handlers[e]->name)) {
		    if (handlers[e]->handler) {
			(handlers[e]->handler)(node);
		    }
		    for (attr=0; (attr<MAX_ATTR_PER_NODE)&&handlers[e]->attrList[attr]; attr++) {
			if (xmlHasProp(node, handlers[e]->attrList[attr]->name)) {	    
			    if (handlers[e]->attrList[attr]->handler)
				(handlers[e]->attrList[attr]->handler)(node, xmlGetProp(node, handlers[e]->attrList[attr]->name));
			}
		    }
		    
		    ProcessXmlTree(node->children, handlers[e]->children);
		}	    
	    }
	}
    }
}

static char *xsdDef[2]={0, 0};

static void ParseXMLFile(const char *xml, const char *xsdFile) {
    xmlSchemaValidCtxtPtr validSchema;
    xmlSchemaParserCtxtPtr xsdParser;
    xmlSchemaPtr schema;
    xmlNodePtr cur;
    struct stat st;
    xmlDocPtr doc;
    
    xmlFile=(char*)xml;
    // TODO: should be more explicit with failing files
    if (!xsdFile||(stat(xsdFile, &st)<0)) {
	// Trying default xsd paths
	if (stat(xsdDef[0], &st)<0) {
	    if (stat(xsdDef[1], &st)<0) {
		ShowErrorMsgAndExit("XSD file not found");
	    } else  {
		xsdFile=xsdDef[1];
	    }
	} else {
	    xsdFile=xsdDef[0];
	}
	
    }
    if (!(xsdParser=xmlSchemaNewParserCtxt(xsdFile)))
	ShowErrorMsgAndExit("Invalid XSD file");
    
    if (!(schema=xmlSchemaParse(xsdParser)))      
	ShowErrorMsgAndExit("XSD file \"%s\" not valid", xsdFile);
    
    validSchema=xmlSchemaNewValidCtxt(schema);
    
    if (!(doc=xmlParseFile(xml)))
	ShowErrorMsgAndExit("XML file \"%s\" not found", xml);
    
    if (xmlSchemaValidateDoc(validSchema, doc))
	ShowErrorMsgAndExit("XML file \"%s\" not valid", xml);
    
    cur=xmlDocGetRootElement(doc);
    ProcessXmlTree(cur, rootHandlers);
    
    xmlSchemaFreeValidCtxt(validSchema);
    xmlSchemaFreeParserCtxt(xsdParser);
    xmlSchemaFree(schema);
    
    xmlFreeDoc(doc);
    xmlCleanupParser();
}

typedef void (*DevInit_t)(void);
static void DevicesInit(void) {
    extern DevInit_t _sdevtab[], _edevtab[];
    xm_s32_t e;
    for (e=0; &_sdevtab[e]<_edevtab; e++) {
	_sdevtab[e]();
    }
    InsertDevAsoc("#");
}

int main(int argc, char **argv)  {
    char *xsdFile=0, *pwd;
    FILE *outFile=stderr;
    char *output=NULL;
    int opt;

    if (argc<2)
	ShowErrorMsgAndExit(USAGE);

    while ((opt = getopt(argc, argv, "o:s:")) != -1) {
        switch (opt) {
        case 'o':
	    if (!(output=malloc(strlen(optarg)+1)))
                ShowErrorMsgAndExit("Dynamic memory pool exhausted");
            strcpy(output, optarg);
            break;
	case 's':
	    xsdFile=malloc(strlen(optarg)+1);
	    strcpy(xsdFile, optarg);
	    break;
        default: /* ? */
            ShowErrorMsgAndExit(USAGE);
        }
    }
    
    if (optind>=argc)
        ShowErrorMsgAndExit(USAGE);
    
    pwd=dirname(strdup(argv[0]));
    xsdDef[0]=malloc(strlen(pwd)+strlen(XSD_DEFAULT_FILE0)+2);
    sprintf(xsdDef[0], "%s/%s", pwd, XSD_DEFAULT_FILE0);
    xsdDef[1]=malloc(strlen(pwd)+strlen(XSD_DEFAULT_FILE1)+2);
    sprintf(xsdDef[1], "%s/%s", pwd, XSD_DEFAULT_FILE1);

    InitXMCDefaults(&xmcTab);

    // initialise libxml library and check potential ABI mismatches
    LIBXML_TEST_VERSION;
    ArchInit(&xmcTab);
    InitConstraints();
    DevicesInit();
    InitHm();
    ParseXMLFile(argv[optind], xsdFile);   
    ProcessXMC(&xmcTab);
    CheckConstraints(&xmcTab);
    if (output) {
	if(!(outFile=fopen(output, "w")))
	    ShowErrorMsgAndExit("Output file \"%s\" cannot be created", output);
    }
    
    PrintXMC(&xmcTab, outFile);
    return 0;
}

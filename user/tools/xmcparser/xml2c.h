/*
 * $FILE: xml2c.h
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XMC_XML2C_H_
#define _XMC_XML2C_H_

typedef void (*dev2cHandler_t)(struct xmc *xmcTab, FILE *outFile);

extern dev2cHandler_t *dev2cTab;
extern void InsertDev2cHandler(dev2cHandler_t handler);
extern void ExecDev2cHandlers(struct xmc *xmcTab, FILE *outFile);
extern void InsertDevTab2cHandler(dev2cHandler_t handler);
extern void ExecDevTab2cHandlers(struct xmc *xmcTab, FILE *outFile);
#define TAB "    "
#define ADD1TAB(x)	TAB x
#define ADD2TAB(x)	TAB ADD1TAB(x)
#define ADD3TAB(x)	TAB ADD2TAB(x)
#define ADD4TAB(x)	TAB ADD3TAB(x)
#define ADD5TAB(x)	TAB ADD4TAB(x)
#define ADD6TAB(x)	TAB ADD5TAB(x)
#define ADD7TAB(x)	TAB ADD6TAB(x)
#define ADD8TAB(x)	TAB ADD7TAB(x)
#define ADD9TAB(x)	TAB ADD8TAB(x)
#define ADD10TAB(x)	TAB ADD9TAB(x)
/* ADDNTAB(n,x), where n is a digit and x a string */
#define ADDNTAB(n,x)	ADD ## n ## TAB(x)

#endif

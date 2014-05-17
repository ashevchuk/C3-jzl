#pragma once

typedef unsigned char uchar;
typedef unsigned int uint;

//Truncate an ANSI template to a given size: 
//
//Input:
//  pSrcTemplate - Original template
//  pTruncatedTemplate - Truncated template, allocated with nMaxTruncatedTemplateSize bytes
//  nMaxTruncatedTemplateSize - Requested maximum size of the template
//
//Output:
//  pTruncatedTemplate - Truncated template
//  nMaxTruncatedTemplateSize - Actual size of the truncated template (can be slightly smaller than the 
//                              requested size)
void TruncateANSITemplate(const uchar* pSrcTemplate, uchar* pTruncatedTemplate, uint * nMaxTruncatedTemplateSize);

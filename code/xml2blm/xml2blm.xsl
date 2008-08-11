<?xml version="1.0" encoding="utf-8"?>
<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!--
	xml2blm
	
	converts blinkenlights movies in XML representation
	to classic blm files.
-->


	<xsl:output method="text" encoding="ascii"/>

	<xsl:template match="blm">
		<xsl:text># BlinkenLights Movie </xsl:text>
		<xsl:value-of select="@width"/>
		<xsl:text>x</xsl:text>
		<xsl:value-of select="@height"/>
		<xsl:text>&#x0a;</xsl:text>
		<xsl:apply-templates select="header/*" mode="header"/>
		<xsl:text>&#x0a;</xsl:text>
		<xsl:apply-templates select="frame" mode="frames"/>
	</xsl:template>

	<xsl:template match="*" mode="header">
		<xsl:text># </xsl:text>		
		<xsl:value-of select="name(.)"/>
		<xsl:text> = </xsl:text>		
		<xsl:value-of select="."/>
		<xsl:text>&#x0a;</xsl:text>
	</xsl:template>

	<xsl:template match="frame" mode="frames">
		<xsl:text>@</xsl:text>		
		<xsl:value-of select="@ms"/>
		<xsl:text>&#x0a;</xsl:text>
		<xsl:apply-templates select="row" mode="frames"/>
		<xsl:text>&#x0a;</xsl:text>
	</xsl:template>

	<xsl:template match="row" mode="frames">
		<xsl:value-of select="."/>
		<xsl:text>&#x0a;</xsl:text>
	</xsl:template>
</xsl:transform>

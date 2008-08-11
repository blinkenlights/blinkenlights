<?xml version="1.0" encoding="utf-8"?>

<!--
	master.xsl

	automatic transformation of web site pages.
-->

<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="xml" encoding="utf-8" indent="yes" />

<xsl:template match="mcu-setup">
	<mcu-setup>
		<xsl:apply-templates select="lamp">
			<xsl:sort select="@row" data-type="number" />
			<xsl:sort select="@column" data-type="number" />
		</xsl:apply-templates>
	</mcu-setup>
</xsl:template>


<xsl:template match="lamp">
	<xsl:copy-of select="." />
</xsl:template>

</xsl:transform>

<?xml version="1.0" encoding="utf-8"?>
<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="xml" encoding="utf-8" indent="yes" />



<xsl:template match="simple-playlist">
	<playlist>
		<xsl:apply-templates select="*" />
	</playlist>
</xsl:template>


<!-- map special elements to generic item elements -->

<xsl:template match="movie" >
	<item type="BMoviePlayer">
		<xsl:apply-templates select="@*" mode="param" />
	</item>
</xsl:template>

<xsl:template match="pong" >
	<item type="BPong">
		<xsl:apply-templates select="@*" mode="param" />
	</item>
</xsl:template>

<xsl:template match="pacman" >
	<item type="BPacman">
		<xsl:apply-templates select="@*" mode="param" />
	</item>
</xsl:template>

<xsl:template match="breakout" >
	<item type="BBreakout">
		<xsl:apply-templates select="@*" mode="param" />
	</item>
</xsl:template>

<xsl:template match="tetris" >
	<item type="BTetris">
		<xsl:apply-templates select="@*" mode="param" />
	</item>
</xsl:template>

<xsl:template match="fire" >
	<item type="BFire">
		<xsl:apply-templates select="@*" mode="param" />
	</item>
</xsl:template>

<!-- standard attribute to param key/value pair conversion -->

<xsl:template match="@*" mode="param" >
	<param key="{name()}" value="{.}" />
</xsl:template>

<!-- parameter renaming -->

<xsl:template match="@file" mode="param" >
	<param key="movie" value="{.}" />
</xsl:template>

</xsl:transform>

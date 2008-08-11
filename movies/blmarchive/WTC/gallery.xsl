<?xml version="1.0" encoding="utf-8"?>
<xsl:transform version="1.1" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="xml" encoding="utf-8" indent="yes"/>

	<xsl:template match="images">
		<html xmlns="http://www.w3.org/1999/xhtml">
			<head>
				<title xml:lang="de">Blinkenlights WTC movies</title>
			</head>

			<body>
				<h1>Blinkenlights WTC movies</h1>

				<div class="gallery">
					<table>
						<xsl:for-each select="image">
							<tr>
								<td>
									<img src="small/{@id}.gif" alt="{@id}"/>
								</td>			
								<td>
									<p>
										<xsl:value-of select="@id"/>:
										<a href="huge/{@id}.gif">huge</a>
										<a href="large/{@id}.gif">large</a>
										<a href="medium/{@id}.gif">medium</a>
										<a href="small/{@id}.gif">small</a>
									</p>
								</td>
							</tr>
						</xsl:for-each>
					</table>
				</div>
			</body>
		</html>
	</xsl:template>

</xsl:transform>

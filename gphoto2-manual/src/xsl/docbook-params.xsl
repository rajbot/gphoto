<?xml version='1.0' encoding='utf-8'?>

<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns="http://www.w3.org/1999/xhtml"
  version="1.0">

  <xsl:param name="use.id.as.filename" select="1"/>
  <xsl:param name="make.valid.html"    select="1"/>
  <xsl:param name="navig.graphics"     select="0"/>
  <xsl:param name="chapter.autolabel"  select="1"/>
  <xsl:param name="section.autolabel"  select="1"/>
  <xsl:param name="l10n.gentext.language" select="'en'"/>
  <xsl:param name="section.label.includes.component.label" select="0"/>
  <xsl:param name="html.stylesheet"    select="'../../styles.css'"/>
  <xsl:param name="qanda.inherit.numeration" select="1"/>

  <xsl:template name="user.header.navigation">
    <xsl:param name="node"/>
    <!-- ==================================================================
         the following <p></p> element was taken from the PHP code of the 
         new gphoto2 web site 
         ================================================================== -->
    <p align="center" class="menu">
      <a href="/">Home</a> :: 
      <a href="/news">News</a> :: 
      <a href="/proj/">Projects</a> :: 
      <a href="/doc/">Documentation</a> :: 
      <a href="http://www.sf.net/projects/gphoto/">Developers</a> :: 
      <a href="/mailinglists/">Mailing lists</a> :: 
      <a href="http://sourceforge.net/project/showfiles.php?group_id=8874&amp;release_id=96632">Download</a> :: 
      <a href="/links/">Links</a>
      <br/>
      <a href="/doc/manual/">User's manual</a> ::
      <a href="/doc/faq/">FAQ</a>
    </p>
  </xsl:template>

</xsl:stylesheet>

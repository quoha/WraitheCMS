<code>
siteName is "<cms siteName />"
pageTitle is "<cms pageTitle />"
</code>

<!-- the ! in the next line should cause articleText to be evaluated as a view -->
<cms articleText? if articleText ! else <this space for rent> endif />

<cms <footer.tpl> include />

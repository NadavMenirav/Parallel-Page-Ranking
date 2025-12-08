# Parallel-Page-Ranking
Well... a parallel implementation of Google's PageRank algorithm.

## What the heck is Page Rank?
When surfing the internet, we need a way to determine which results are the most relevant to the user. In order to so we need to be able to RANK webPAGEs, hence PageRank. The jist of it, is that the more a website is "cited", or linked to, the more it is relevant. From here, the rest of the logic is destined to determine which links are more worthwhile, and credit the website to which they link with a higher rank.

The algorithm which calculates how "cited" a website is, emulates a random surfer travelling between websites via weblinks or entering search queries in different probabilities.

## Notations
* $d := 0.85$ (the damping factor) the probability that the user continues surfing (moving from one website to another via a link).
* $1-d$ the probability that the user makes a new search instead of surfing.
* $N$ - the total number of websites.
* $u_i$ - the websites that link to a site $v$.
* $u_j$ - the websites that don't link to any other websites.
* $\text{Outlink}(v)$ - the number of websites to which $v$ links.

## The formula
$$\text{PageRank}(v) := \frac{1-d}{N} + d \cdot \left(\sum_{u_i} \frac{\text{PageRank}(u_i)}{\text{Outlink}(u_i)}+ \sum_{u_j} \frac{\text{PageRank}(u_j)}{N}\right)
$$


* Each element in the left sum, represents the importance of the link from $u_i$ to $v$. The more important $u_i$ is, the more important the link is. Additionally, if $u_i$ points to a ton of websites, the importance of $u_i$'s links are split amongst all the websites to which it points.
* The second sum is meant to consider the cases in which we have arrived at a website that has no outlinks, and hence we'd have to make a new search in order to continue.

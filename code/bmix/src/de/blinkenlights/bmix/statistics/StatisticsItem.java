package de.blinkenlights.bmix.statistics;

import java.io.Serializable;
import java.util.List;

public interface StatisticsItem extends Serializable {

	public String getName();
	
	public List<StatisticsItem> getChildren();
	
	public String toHtml();
}

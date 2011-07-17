
package pokerth_protocol;
//
// This file was generated by the BinaryNotes compiler.
// See http://bnotes.sourceforge.net 
// Any modifications to this file will be lost upon recompilation of the source ASN.1. 
//

import org.bn.*;
import org.bn.annotations.*;
import org.bn.annotations.constraints.*;
import org.bn.coders.*;
import org.bn.types.*;




    @ASN1PreparedElement
    @ASN1Sequence ( name = "GameStartModeRejoin", isSet = false )
    public class GameStartModeRejoin implements IASN1PreparedElement {
            
@ASN1SequenceOf( name = "rejoinPlayerData", isSetOf = false ) 

    @ASN1ValueRangeConstraint ( 
		
		min = 2L, 
		
		max = 10L 
		
	   )
	   
        @ASN1Element ( name = "rejoinPlayerData", isOptional =  false , hasTag =  false  , hasDefaultValue =  false  )
    
	private java.util.Collection<RejoinPlayerData>  rejoinPlayerData = null;
                
  
        
        public java.util.Collection<RejoinPlayerData>  getRejoinPlayerData () {
            return this.rejoinPlayerData;
        }

        

        public void setRejoinPlayerData (java.util.Collection<RejoinPlayerData>  value) {
            this.rejoinPlayerData = value;
        }
        
  
                    
        
        public void initWithDefaults() {
            
        }

        private static IASN1PreparedElementData preparedData = CoderFactory.getInstance().newPreparedElementData(GameStartModeRejoin.class);
        public IASN1PreparedElementData getPreparedData() {
            return preparedData;
        }

            
    }
            

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
    @ASN1Sequence ( name = "RejoinExistingGame", isSet = false )
    public class RejoinExistingGame implements IASN1PreparedElement {
            
        @ASN1Element ( name = "gameId", isOptional =  false , hasTag =  false  , hasDefaultValue =  false  )
    
	private NonZeroId gameId = null;
                
  
        
        public NonZeroId getGameId () {
            return this.gameId;
        }

        

        public void setGameId (NonZeroId value) {
            this.gameId = value;
        }
        
  
                    
        
        public void initWithDefaults() {
            
        }

        private static IASN1PreparedElementData preparedData = CoderFactory.getInstance().newPreparedElementData(RejoinExistingGame.class);
        public IASN1PreparedElementData getPreparedData() {
            return preparedData;
        }

            
    }
            
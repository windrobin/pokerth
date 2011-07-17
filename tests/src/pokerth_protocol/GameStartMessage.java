
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
    @ASN1BoxedType ( name = "GameStartMessage" )
    public class GameStartMessage implements IASN1PreparedElement {
                
        

       @ASN1PreparedElement
       @ASN1Sequence ( name = "GameStartMessage" , isSet = false )
       public static class GameStartMessageSequenceType implements IASN1PreparedElement {
                
        @ASN1Element ( name = "gameId", isOptional =  false , hasTag =  false  , hasDefaultValue =  false  )
    
	private NonZeroId gameId = null;
                
  
        @ASN1Element ( name = "startDealerPlayerId", isOptional =  false , hasTag =  false  , hasDefaultValue =  false  )
    
	private NonZeroId startDealerPlayerId = null;
                
  
        
    @ASN1PreparedElement
    @ASN1Choice ( name = "gameStartMode" )
    public static class GameStartModeChoiceType implements IASN1PreparedElement {
            
        @ASN1Element ( name = "gameStartModeInitial", isOptional =  false , hasTag =  true, tag = 0 , hasDefaultValue =  false  )
    
	private GameStartModeInitial gameStartModeInitial = null;
                
  
        @ASN1Element ( name = "gameStartModeRejoin", isOptional =  false , hasTag =  true, tag = 1 , hasDefaultValue =  false  )
    
	private GameStartModeRejoin gameStartModeRejoin = null;
                
  
        
        public GameStartModeInitial getGameStartModeInitial () {
            return this.gameStartModeInitial;
        }

        public boolean isGameStartModeInitialSelected () {
            return this.gameStartModeInitial != null;
        }

        private void setGameStartModeInitial (GameStartModeInitial value) {
            this.gameStartModeInitial = value;
        }

        
        public void selectGameStartModeInitial (GameStartModeInitial value) {
            this.gameStartModeInitial = value;
            
                    setGameStartModeRejoin(null);
                            
        }

        
  
        
        public GameStartModeRejoin getGameStartModeRejoin () {
            return this.gameStartModeRejoin;
        }

        public boolean isGameStartModeRejoinSelected () {
            return this.gameStartModeRejoin != null;
        }

        private void setGameStartModeRejoin (GameStartModeRejoin value) {
            this.gameStartModeRejoin = value;
        }

        
        public void selectGameStartModeRejoin (GameStartModeRejoin value) {
            this.gameStartModeRejoin = value;
            
                    setGameStartModeInitial(null);
                            
        }

        
  

	    public void initWithDefaults() {
	    }

        public IASN1PreparedElementData getPreparedData() {
            return preparedData_GameStartModeChoiceType;
        }

        private static IASN1PreparedElementData preparedData_GameStartModeChoiceType = CoderFactory.getInstance().newPreparedElementData(GameStartModeChoiceType.class);

    }

                
        @ASN1Element ( name = "gameStartMode", isOptional =  false , hasTag =  false  , hasDefaultValue =  false  )
    
	private GameStartModeChoiceType gameStartMode = null;
                
  
        
        public NonZeroId getGameId () {
            return this.gameId;
        }

        

        public void setGameId (NonZeroId value) {
            this.gameId = value;
        }
        
  
        
        public NonZeroId getStartDealerPlayerId () {
            return this.startDealerPlayerId;
        }

        

        public void setStartDealerPlayerId (NonZeroId value) {
            this.startDealerPlayerId = value;
        }
        
  
        
        public GameStartModeChoiceType getGameStartMode () {
            return this.gameStartMode;
        }

        

        public void setGameStartMode (GameStartModeChoiceType value) {
            this.gameStartMode = value;
        }
        
  
                
                
        public void initWithDefaults() {
            
        }

        public IASN1PreparedElementData getPreparedData() {
            return preparedData_GameStartMessageSequenceType;
        }

       private static IASN1PreparedElementData preparedData_GameStartMessageSequenceType = CoderFactory.getInstance().newPreparedElementData(GameStartMessageSequenceType.class);
                
       }

       
                
        @ASN1Element ( name = "GameStartMessage", isOptional =  false , hasTag =  true, tag = 22, 
        tagClass =  TagClass.Application  , hasDefaultValue =  false  )
    
        private GameStartMessageSequenceType  value;        

        
        
        public GameStartMessage () {
        }
        
        
        
        public void setValue(GameStartMessageSequenceType value) {
            this.value = value;
        }
        
        
        
        public GameStartMessageSequenceType getValue() {
            return this.value;
        }            
        

	    public void initWithDefaults() {
	    }

        private static IASN1PreparedElementData preparedData = CoderFactory.getInstance().newPreparedElementData(GameStartMessage.class);
        public IASN1PreparedElementData getPreparedData() {
            return preparedData;
        }

            
    }
            